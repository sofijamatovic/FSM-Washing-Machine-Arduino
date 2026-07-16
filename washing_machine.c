#include <stdio.h>
//everything is in serbian because it was a project for my college, have fun :)
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

/*
 * u pravom uredjaju ovo bi dolazilo od senzora i tastera, u
 * ovoj verziji programa dogadjaje rucno zadajemo u main() funkciji.
 */

typedef enum {
    DOG_START,
    DOG_VRATA_ZATVORENA,
    DOG_NIVO_VODE_OK,
    DOG_VREME_ISTEKLO,
    DOG_ISPUSTANJE_ZAVRSENO,
    DOG_RESET,
    DOG_VRATA_OTVORENA
} Dogadjaj;

//trajanje faza u zavisnosti od vremena
#define TRAJANJE_PRANJA      3
#define TRAJANJE_ISPIRANJA   2
#define TRAJANJE_CENTRIFUGE  2

typedef struct {
    Stanje stanje; //u kom je trenutno
    int broj_ispiranja; //br ispiranja koji je vec odradjen u ovom ciklusu
    int ciljni_broj_ispiranja; //br ispiranja koji treba da se odradi
    int preostalo_vreme; //koliko jos treba da prodje u ovoj fazi
} VesMasina;

void vesmasina_init(VesMasina* vm) { //vm pokazivac na celu strukturu
    //postavlja na ove nulte vrednosti ves masinu
    vm->stanje = STANJE_MIROVANJE;
    vm->broj_ispiranja = 0;
    vm->ciljni_broj_ispiranja = 2;
    vm->preostalo_vreme = 0;
}

void vesmasina_azuriraj(VesMasina* vm, Dogadjaj dog) { //ovde se menja stanje ves masine,
    //dogadjaj prima dogadjaj koji je pristigao upravo

    /* provera greske ima prioritet i vazi za sve aktivne faze rada */
    if (dog == DOG_VRATA_OTVORENA &&
        (vm->stanje == STANJE_PUNJENJE || vm->stanje == STANJE_PRANJE ||
            vm->stanje == STANJE_ISPIRANJE || vm->stanje == STANJE_CENTRIFUGA)) {
        vm->stanje = STANJE_GRESKA;
        return;
    }

    switch (vm->stanje) {

    case STANJE_MIROVANJE:
        if (dog == DOG_START)
            vm->stanje = STANJE_ZAKLJUCAVANJE_VRATA;
        break;

    case STANJE_ZAKLJUCAVANJE_VRATA:
        if (dog == DOG_VRATA_ZATVORENA)
            vm->stanje = STANJE_PUNJENJE;
        break;

    case STANJE_PUNJENJE:
        if (dog == DOG_NIVO_VODE_OK) {
            vm->stanje = STANJE_PRANJE;
            vm->preostalo_vreme = TRAJANJE_PRANJA;
        }
        break;

    case STANJE_PRANJE:
        if (dog == DOG_VREME_ISTEKLO)
            vm->stanje = STANJE_ISPUSTANJE;
        break;

    case STANJE_ISPUSTANJE: //ovde se proverava dodatno i da li je uradjen dovoljan br ispiranja ako nije, 
        //br_ispiranja se povecava za 1 i masina opet ispira, ako jeste prelazi na centrifugu

        if (dog == DOG_ISPUSTANJE_ZAVRSENO) {
            if (vm->broj_ispiranja < vm->ciljni_broj_ispiranja) {
                vm->broj_ispiranja++;
                vm->stanje = STANJE_ISPIRANJE;
                vm->preostalo_vreme = TRAJANJE_ISPIRANJA;
            }
            else {
                vm->stanje = STANJE_CENTRIFUGA;
                vm->preostalo_vreme = TRAJANJE_CENTRIFUGE;
            }
        }
        break;

    case STANJE_ISPIRANJE:
        if (dog == DOG_VREME_ISTEKLO)
            vm->stanje = STANJE_ISPUSTANJE;
        break;

    case STANJE_CENTRIFUGA:
        if (dog == DOG_VREME_ISTEKLO)
            vm->stanje = STANJE_KRAJ;
        break;

    case STANJE_KRAJ:
        if (dog == DOG_RESET)
            vm->stanje = STANJE_MIROVANJE;
        break;

    case STANJE_GRESKA:
        break;
    }
}

void vesmasina_vreme(VesMasina* vm) {  //simulira protok neke zadate jedinice vremena. 
    //stanja koja zavise od vremena su pranje ispiranje i centrifuga. 
    //ako preostalo vreme nije isteklo smanjuje se za 1. kad vreme istekne 
    //fju dog vreme istaklo obradjuje vesmasina azuriraj


    if (vm->preostalo_vreme > 0) {
        vm->preostalo_vreme--;
        if (vm->preostalo_vreme == 0) {
            vesmasina_azuriraj(vm, DOG_VREME_ISTEKLO);
        }
    }
}

const char* naziv_stanja(Stanje s) { //samo da bi u ispisu bio tekst a ne brojevi
    switch (s) {
    case STANJE_MIROVANJE:           return "MIROVANJE";
    case STANJE_ZAKLJUCAVANJE_VRATA: return "ZAKLJUCAVANJE_VRATA";
    case STANJE_PUNJENJE:            return "PUNJENJE";
    case STANJE_PRANJE:              return "PRANJE";
    case STANJE_ISPUSTANJE:          return "ISPUSTANJE";
    case STANJE_ISPIRANJE:           return "ISPIRANJE";
    case STANJE_CENTRIFUGA:          return "CENTRIFUGA";
    case STANJE_KRAJ:                return "KRAJ";
    case STANJE_GRESKA:              return "GRESKA";
    }
    return "NEPOZNATO";
}

/* glavna petlja izdvojena iz main() - prima masinu i vodi je kroz ciklus */
void pokreni_ciklus(VesMasina* vm) {
    while (vm->stanje != STANJE_KRAJ && vm->stanje != STANJE_GRESKA) {
        switch (vm->stanje) {
        case STANJE_MIROVANJE:
            vesmasina_azuriraj(vm, DOG_START);
            break;
        case STANJE_ZAKLJUCAVANJE_VRATA:
            vesmasina_azuriraj(vm, DOG_VRATA_ZATVORENA);
            break;
        case STANJE_PUNJENJE:
            vesmasina_azuriraj(vm, DOG_NIVO_VODE_OK);
            break;
        case STANJE_ISPUSTANJE:
            vesmasina_azuriraj(vm, DOG_ISPUSTANJE_ZAVRSENO);
            break;
        case STANJE_PRANJE:
        case STANJE_ISPIRANJE:
        case STANJE_CENTRIFUGA:
            vesmasina_vreme(vm);
            break;
        default:
            break;
        }
        printf("%s (preostalo_vreme = %d)\n", naziv_stanja(vm->stanje), vm->preostalo_vreme);
    }
}

int main(void) {

    VesMasina vm; //nasa ves masina
    vesmasina_init(&vm);

    printf("%s\n", naziv_stanja(vm.stanje));

    pokreni_ciklus(&vm);

    if (vm.stanje == STANJE_GRESKA) {
        printf("PROGRAM ZAUSTAVLJEN ZBOG GRESKE\n");
        return 1;
    }

    vesmasina_azuriraj(&vm, DOG_RESET);
    printf("%s\n", naziv_stanja(vm.stanje));

    return 0;
}
