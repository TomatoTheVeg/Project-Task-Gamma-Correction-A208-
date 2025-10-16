#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <float.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include "pnm_image.h"
#include "benchmark.h"

#define no_argument 0
#define required_argument 1

extern void gamma_correct_V2(
    const uint8_t *img, size_t width, size_t height,
    float a, float b, float c,
    float gamma,
    uint8_t *result);

extern void gamma_correct_V1(
    const uint8_t *img, size_t width, size_t height,
    float a, float b, float c,
    float gamma,
    uint8_t *result);

extern void gamma_correct(
    const uint8_t *img, size_t width, size_t height,
    float a, float b, float c,
    float gamma,
    uint8_t *result);

const gamma_func_ptr gammaFunctionArray[] = {
    gamma_correct, gamma_correct_V1, gamma_correct_V2
};


const char *help_msg =
    "Help Message\n\n"
    "Benoetigte Argumente:\n"
    "-o <Dateiname> — Ausgabedatei\n"
    "--gamma <Floating Point Zahl> — γ ∈ [0, ∞)\n"
    "<Dateiname> — Positional Argument: Eingabedatei\n\n"
    "Optionale Argumente:\n"
    "-V <Zahl> — Die Implementierung, die verwendet wird. -V 0 verwendet die Hauptimplementierung. Wenn diese Option nicht gesetzt wird, wird ebenfalls die Hauptimplementierung ausgefuehrt.\n"
    "-B<Zahl> — Falls gesetzt, wird die Laufzeit der angegebenen Implementierung gemessen und ausgegeben. Das optionale Argument dieser Option gibt die Anzahl an Wiederholungen des Funktionsaufrufs an, z. B. -B oder -B5 .\n"
    "--coeffs <FP Zahl>,<FP Zahl>,<FP Zahl> — Die Koeffizienten der Graustufenkonvertierung a, b und c.\n"
    "–h|--help — Zeigt eine Beschreibung aller Optionen des Programms sowie Verwendungsbeispiele an.\n\n"
    "Verwendungsbeispiele:\n";

void print_help(char *argv0)
{
    printf("%s", help_msg);
    printf("Ohne optionale Argumente: %s -o img.pgm --gamma 0.5 img.ppm\n", argv0);
    printf("Mit optionalen Argumenten: %s -V 1 -B5 -o img.pgm --coeffs 1.2,0.3,1 --gamma 0.5 img.ppm\n", argv0);
}

void print_usage(char *argv0)
{
    fprintf(stderr, "\nUsage: %s [OPTIONS]... [FILE]\n", argv0);
    fprintf(stderr, "Try --help or -h for more info");
}

int main(int argc, char *argv[])
{
    struct option long_options[] = {
        {"gamma", required_argument, 0, 'g'},
        {"coeffs", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    int opt;

    int implementation = 0;
    int repetitions = -1;
    int long_index = 0;

    char *output_file = "";
    char *input_file = "";

    // Adobe RGB values
    float coeffs[3] = {0.2125, 0.7154, 0.0721};
    float gamma = -1;

    errno = 0; // reset errno before the operation
    char *endptr;

    while ((opt = getopt_long(argc, argv, "V:B::o:h", long_options, &long_index)) != -1)
    {
        switch (opt)
        {
            //-V version
        case 'V':
            implementation = strtol(optarg, &endptr, 10); // using strtol for safety here and in other code
            size_t arrayLength = sizeof(gammaFunctionArray) / sizeof(gammaFunctionArray[0]);
            if (errno != 0)
            {
                perror("Ein Fehler ist aufgetretten");
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            // version shoul be smaller than array length
            else if (endptr == optarg || *endptr != '\0' || implementation >= (int)arrayLength || implementation < 0)
            {
                fprintf(stderr, "Fehler: Ungueltige Eingabe von Implementierungsnummer: %s\nMuss im Bereich [0;%lu] sein.\n", optarg, arrayLength - 1);
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            break;
        case 'B': //-B repetitons and benchmarking
            if (optarg)
            {
                repetitions = strtol(optarg, &endptr, 10);
                if (errno != 0)
                {
                    perror("Ein Fehler ist aufgetretten");
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                else if (endptr == optarg || *endptr != '\0' || errno == ERANGE || repetitions <= 0)
                {
                    fprintf(stderr, "Fehler: Ungueltige Eingabe von Wiederholungsanzahl: %s\nMuss groesser als 0 sein und kleiner als %d.\n", optarg, INT32_MAX);
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
            }
            else
            {
                repetitions = 1;
            }
            break;
        case 'o': //-o output file
            output_file = optarg;
            break;
        case 'c': //--coeffs
        {
            char *token;
            int i = 0;
            token = strtok(optarg, ",");
            while (token != NULL && i < 3)
            {
                errno = 0;
                coeffs[i] = strtof(token, &endptr); // parse coeffs by one
                if (errno != 0)
                {
                    perror("Ein Fehler ist aufgetretten");
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                else if (endptr == token || *endptr != '\0' || errno == ERANGE)
                {
                    fprintf(stderr, "Fehler: Ungueltige Eingabe fuer Koeffizient: %s\n", token);
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                i++;
                token = strtok(NULL, ",");
            }

            if (i != 3)
            {
                fprintf(stderr, "Fehler: Es muessen genau 3 Koeffizienten angegeben werden\n");
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            break;
        }
        case 'g': //--gamma γ ∈ [0, ∞)
        {
            gamma = strtof(optarg, &endptr);
            if (errno != 0)
            {
                perror("Ein Fehler ist aufgetretten");
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            else if (endptr == optarg || *endptr != '\0' || errno == ERANGE || gamma < 0)
            {
                fprintf(stderr, "Fehler: Ungueltige Eingabe von Gamma: %s\nMuss groesser als 0 sein und kleiner als %e.\n", optarg, FLT_MAX);
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
        }
        break;
        case 'h': // -h or --help
            print_help(argv[0]);
            return EXIT_SUCCESS;
        default:
            if (argv[optind - 1][1] == '-')
            { // after getopt_long processes an option, it is already moved to the next position -> we use optind - 1
                fprintf(stderr, "Fehler: Unbekannte Option '%s'\n", argv[optind - 1]);
            }
            else if (optopt) // one unrecogized character
            {
                fprintf(stderr, "Fehler: Unbekannte Option '-%c'\n", optopt);
            }
            else
            {
                fprintf(stderr, "Fehler: Unbekannte Option\n");
            }
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    // handle positional argument
    if (optind < argc)
    {
        input_file = argv[optind];
        optind++;
    }
    else
    {
        fprintf(stderr, "Fehler: kein Dateiname ist uebergeben\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // check other arguments
    if (optind < argc)
    {
        fprintf(stderr, "Fehler: Zu viele Argumente uebergeben: ");
        while (optind < argc)
        {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strlen(output_file) == 0) // check if output file is set
    {
        fprintf(stderr, "Fehler: keine Ausgabedatei eingegeben\n");
        if (strlen(input_file) == 0)
        {
            fprintf(stderr, "Fehler: kein Dateiname ist uebergeben\n");
        }
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    if(gamma < 0) { 
        fprintf(stderr, "Fehler: kein gamma-Wert eingegeben\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    PNMImage *img = load_ppm_p6(input_file);
    if (!img)
    {
        fprintf(stderr, "Fehler: Bild konnte nicht geladen werden.\n");
        return EXIT_FAILURE;
    }

    PNMImage *pgm = create_grayscale_pgm(img);
    if (!pgm)
    {
        fprintf(stderr, "Fehler: Fehler beim Erstellen des Graustufenbildes.\n");
        free(img->data);
        free(img);
        return EXIT_FAILURE;
    }

    // set chosen function
    gamma_func_ptr selected_func = gammaFunctionArray[implementation];
    
    if (!selected_func)
    {
        size_t arrayLength = sizeof(gammaFunctionArray) / sizeof(gammaFunctionArray[0]);
        fprintf(stderr, "Fehler: Ungueltige Eingabe von Implementierungsanzahl: %s\nMuss groesser oder glech 0 sein und kleiner als %lu.\n", optarg, arrayLength);
        print_usage(argv[0]);

        free(img->data);
        free(img);
        free(pgm->data);
        free(pgm);
        return EXIT_FAILURE;
    }

    // set repetitions ans use benchmark if set
    if (repetitions > 0)
    {
        double s = benchmark(repetitions, img->data, img->width, img->height, coeffs[0], coeffs[1], coeffs[2], gamma, pgm->data, selected_func);
        printf("gamma_correct_V%d: %fs\n", implementation, s);
    }
    else
    {
        selected_func(img->data, img->width, img->height, coeffs[0], coeffs[1], coeffs[2], gamma, pgm->data);
    }

    writePGMP5(output_file, pgm);

    // free allocated memory
    free(img->data);
    free(img);

    free(pgm->data);
    free(pgm);

    return EXIT_SUCCESS;
}
