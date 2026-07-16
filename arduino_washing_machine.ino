#include <LiquidCrystal_I2C.h>

// 1. DEFINICIJA ENUMA I STRUKTURA
typedef enum {
    STANJE_MIROVANJE,
    STANJE_ZAKLJUCAVANJE_VRATA,
    STANJE_PUNJENJE,
    STANJE_PRANJE,
    STANJE_ISPUSTANJE,
    STANJE_ISPIRANJE,
    STANJE_CENTRIFUGA,
    STANJE_KRAJ,
    STANJE_GRESKA
} Stanje;

typedef enum {
    DOG_NONE,
    DOG_START,
    DOG_VRATA_ZATVORENA,
    DOG_NIVO_VODE_OK,
    DOG_VREME_ISTEKLO,
    DOG_ISPUSTANJE_ZAVRSENO,
    DOG_RESET,
    DOG_VRATA_OTVORENA
} Dogadjaj;

#define TRAJANJE_PRANJA      5 
#define TRAJANJE_ISPIRANJA   4
#define TRAJANJE_CENTRIFUGE  4

typedef struct {
    Stanje stanje; 
    int broj_ispiranja; 
    int ciljni_broj_ispiranja; 
    int preostalo_vreme; 
} VesMasina;

// 2. DEFINICIJA PINOVA ZA HARDVER
#define PIN_TASTER_START 2
#define PIN_TASTER_VRATA 3
#define PIN_TASTER_VODA  4

#define PIN_LED_PUMPA    12
#define PIN_LED_MOTOR    11

// Inicijalizacija ekrana na I2C adresi 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

VesMasina vm;
unsigned long prethodno_vreme = 0;

// 3. LOGIKA TVOG FSM-A (TVOJ C KOD)
void vesmasina_init(VesMasina* vm_ptr) { 
    vm_ptr->stanje = STANJE_MIROVANJE;
    vm_ptr->broj_ispiranja = 0;
    vm_ptr->ciljni_broj_ispiranja = 2;
    vm_ptr->preostalo_vreme = 0;
}

void vesmasina_azuriraj(VesMasina* vm_ptr, Dogadjaj dog) { 
    if (dog == DOG_NONE) return;

    /* Provera greške (otvorena vrata u toku rada) */
    if (dog == DOG_VRATA_OTVORENA &&
        (vm_ptr->stanje == STANJE_PUNJENJE || vm_ptr->stanje == STANJE_PRANJE ||
            vm_ptr->stanje == STANJE_ISPIRANJE || vm_ptr->stanje == STANJE_CENTRIFUGA)) {
        vm_ptr->stanje = STANJE_GRESKA;
        return;
    }

    switch (vm_ptr->stanje) {
    case STANJE_MIROVANJE:
        if (dog == DOG_START) {
            vm_ptr->stanje = STANJE_ZAKLJUCAVANJE_VRATA;
            vm_ptr->broj_ispiranja = 0;
        }
        break;

    case STANJE_ZAKLJUCAVANJE_VRATA:
        if (dog == DOG_VRATA_ZATVORENA)
            vm_ptr->stanje = STANJE_PUNJENJE;
        break;

    case STANJE_PUNJENJE:
        if (dog == DOG_NIVO_VODE_OK) {
            vm_ptr->stanje = STANJE_PRANJE;
            vm_ptr->preostalo_vreme = TRAJANJE_PRANJA;
        }
        break;

    case STANJE_PRANJE:
        if (dog == DOG_VREME_ISTEKLO) {
            vm_ptr->stanje = STANJE_ISPUSTANJE;
            vm_ptr->preostalo_vreme = 3; 
        }
        break;

    case STANJE_ISPUSTANJE: 
        if (dog == DOG_ISPUSTANJE_ZAVRSENO || (vm_ptr->preostalo_vreme == 0 && dog == DOG_VREME_ISTEKLO)) {
            if (vm_ptr->broj_ispiranja < vm_ptr->ciljni_broj_ispiranja) {
                vm_ptr->broj_ispiranja++;
                vm_ptr->stanje = STANJE_ISPIRANJE;
                vm_ptr->preostalo_vreme = TRAJANJE_ISPIRANJA;
            }
            else {
                vm_ptr->stanje = STANJE_CENTRIFUGA;
                vm_ptr->preostalo_vreme = TRAJANJE_CENTRIFUGE;
            }
        }
        break;

    case STANJE_ISPIRANJE:
        if (dog == DOG_VREME_ISTEKLO) {
            vm_ptr->stanje = STANJE_ISPUSTANJE;
            vm_ptr->preostalo_vreme = 3;
        }
        break;

    case STANJE_CENTRIFUGA:
        if (dog == DOG_VREME_ISTEKLO)
            vm_ptr->stanje = STANJE_KRAJ;
        break;

    case STANJE_KRAJ:
        if (dog == DOG_RESET || dog == DOG_START)
            vm_ptr->stanje = STANJE_MIROVANJE;
        break;

    case STANJE_GRESKA:
        if (dog == DOG_RESET)
            vesmasina_init(vm_ptr);
        break;
    }
}

void vesmasina_vreme(VesMasina* vm_ptr) {  
    if (vm_ptr->preostalo_vreme > 0) {
        vm_ptr->preostalo_vreme--;
        if (vm_ptr->preostalo_vreme == 0) {
            vesmasina_azuriraj(vm_ptr, DOG_VREME_ISTEKLO);
        }
    }
}

const char* naziv_stanja_lcd(Stanje s) {
    switch (s) {
    case STANJE_MIROVANJE:           return "MIROVANJE";
    case STANJE_ZAKLJUCAVANJE_VRATA: return "ZAKLJ. VRATA";
    case STANJE_PUNJENJE:            return "PUNJENJE VODE";
    case STANJE_PRANJE:              return "PRANJE...";
    case STANJE_ISPUSTANJE:          return "ISPUSTANJE VODE";
    case STANJE_ISPIRANJE:           return "ISPIRANJE...";
    case STANJE_CENTRIFUGA:          return "CENTRIFUGA!";
    case STANJE_KRAJ:                return "KRAJ PROGRAMA";
    case STANJE_GRESKA:              return "GRESKA! VRATA!";
    }
    return "NEPOZNATO";
}

// 4. ARDUINO INICIJALIZACIJA (SETUP)
void setup() {
    lcd.init();
    lcd.backlight();
    
    pinMode(PIN_TASTER_START, INPUT_PULLUP);
    pinMode(PIN_TASTER_VRATA, INPUT_PULLUP);
    pinMode(PIN_TASTER_VODA, INPUT_PULLUP);
    
    pinMode(PIN_LED_PUMPA, OUTPUT);
    pinMode(PIN_LED_MOTOR, OUTPUT);
    
    vesmasina_init(&vm);
}

// 5. ARDUINO GLAVNA PETLJA (LOOP)
void loop() {
    Dogadjaj trenutni_dogadjaj = DOG_NONE;
    unsigned long trenutno_vreme = millis();

    // Čitanje tastera sa debouncing-om (protiv treperenja)
    if (digitalRead(PIN_TASTER_START) == LOW) {
        delay(150);
        while(digitalRead(PIN_TASTER_START) == LOW);
        
        if (vm.stanje == STANJE_MIROVANJE) {
            trenutni_dogadjaj = DOG_START;
        } else if (vm.stanje == STANJE_KRAJ || vm.stanje == STANJE_GRESKA) {
            trenutni_dogadjaj = DOG_RESET;
        }
    }
    
    if (digitalRead(PIN_TASTER_VRATA) == LOW) {
        delay(150);
        while(digitalRead(PIN_TASTER_VRATA) == LOW);
        
        if (vm.stanje == STANJE_ZAKLJUCAVANJE_VRATA) {
            trenutni_dogadjaj = DOG_VRATA_ZATVORENA;
        } else if (vm.stanje == STANJE_PUNJENJE || vm.stanje == STANJE_PRANJE || 
                   vm.stanje == STANJE_ISPIRANJE || vm.stanje == STANJE_CENTRIFUGA) {
            trenutni_dogadjaj = DOG_VRATA_OTVORENA; 
        }
    }

    if (digitalRead(PIN_TASTER_VODA) == LOW) {
        delay(150);
        while(digitalRead(PIN_TASTER_VODA) == LOW);
        
        if (vm.stanje == STANJE_PUNJENJE) {
            trenutni_dogadjaj = DOG_NIVO_VODE_OK;
        }
    }

    // Ažuriranje FSM-a
    if (trenutni_dogadjaj != DOG_NONE) {
        vesmasina_azuriraj(&vm, trenutni_dogadjaj);
    }

    // Odbrojavanje vremena (1 sekunda)
    if (trenutno_vreme - prethodno_vreme >= 1000) {
        prethodno_vreme = trenutno_vreme;
        
        if (vm.stanje == STANJE_PRANJE || vm.stanje == STANJE_ISPIRANJE || 
            vm.stanje == STANJE_CENTRIFUGA || vm.stanje == STANJE_ISPUSTANJE) {
            vesmasina_vreme(&vm);
        }
    }

    // Paljenje i gašenje LED dioda
    if (vm.stanje == STANJE_PUNJENJE || vm.stanje == STANJE_ISPUSTANJE) {
        digitalWrite(PIN_LED_PUMPA, HIGH);
        digitalWrite(PIN_LED_MOTOR, LOW);
    } else if (vm.stanje == STANJE_PRANJE || vm.stanje == STANJE_ISPIRANJE || vm.stanje == STANJE_CENTRIFUGA) {
        digitalWrite(PIN_LED_PUMPA, LOW);
        digitalWrite(PIN_LED_MOTOR, HIGH);
    } else {
        digitalWrite(PIN_LED_PUMPA, LOW);
        digitalWrite(PIN_LED_MOTOR, LOW);
    }

    // Pametno osvežavanje ekrana bez treperenja
    static Stanje prethodno_stanje_prikaz = STANJE_GRESKA;
    static int prethodno_vreme_prikaz = -1;
    static int prethodni_isp_prikaz = -1;

    if (vm.stanje != prethodno_stanje_prikaz || vm.preostalo_vreme != prethodno_vreme_prikaz || vm.broj_ispiranja != prethodni_isp_prikaz) {
        lcd.clear();
        
        lcd.setCursor(0, 0);
        lcd.print("STANJE: ");
        lcd.print(naziv_stanja_lcd(vm.stanje));

        lcd.setCursor(0, 1);
        lcd.print("Vrem:");
        lcd.print(vm.preostalo_vreme);
        lcd.print("s  Isp:");
        lcd.print(vm.broj_ispiranja);
        
        prethodno_stanje_prikaz = vm.stanje;
        prethodno_vreme_prikaz = vm.preostalo_vreme;
        prethodni_isp_prikaz = vm.broj_ispiranja;
    }

    delay(20);
}
