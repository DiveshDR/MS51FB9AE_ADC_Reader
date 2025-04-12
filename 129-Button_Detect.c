#include "MS51_16K.H"

// Define segment pins
sbit SEG_A = P1^3;  // Segment A (P1.3)
sbit SEG_B = P0^0;  // Segment B (P0.0)
sbit SEG_C = P1^5;  // Segment C (P1.5)
sbit SEG_D = P0^3;  // Segment D (P0.3)
sbit SEG_E = P0^1;  // Segment E (P0.1)
sbit SEG_F = P1^2;  // Segment F (P1.2)
sbit SEG_G = P1^7;  // Segment G (P1.7)
sbit SEG_DP = P0^4; // Decimal Point (P0.4)

// Define digit common pins
sbit DIGIT1 = P1^4; // Common Anode 1 (P1.4)
sbit DIGIT2 = P1^1; // Common Anode 2 (P1.1)
sbit DIGIT3 = P1^0; // Common Anode 3 (P1.0)

// Delay function (tuned for ~16MHz)
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 160; j++);
}

// Function to set segments based on a character
void set_segments(char c) {
    // Turn off all segments
    SEG_A = 1; SEG_B = 1; SEG_C = 1; SEG_D = 1;
    SEG_E = 1; SEG_F = 1; SEG_G = 1; SEG_DP = 1;

    // Set patterns (0–9, A, F, I, L, M, O, R, T)
    switch (c) {
        case '0': SEG_A = 0; SEG_B = 0; SEG_C = 0; SEG_D = 0; SEG_E = 0; SEG_F = 0; break;
        case '1': SEG_B = 0; SEG_C = 0; break;
        case '2': SEG_A = 0; SEG_B = 0; SEG_D = 0; SEG_E = 0; SEG_G = 0; break;
        case '3': SEG_A = 0; SEG_B = 0; SEG_C = 0; SEG_D = 0; SEG_G = 0; break;
        case '4': SEG_B = 0; SEG_C = 0; SEG_F = 0; SEG_G = 0; break;
        case '5': SEG_A = 0; SEG_C = 0; SEG_D = 0; SEG_F = 0; SEG_G = 0; break;
        case '6': SEG_A = 0; SEG_C = 0; SEG_D = 0; SEG_E = 0; SEG_F = 0; SEG_G = 0; break;
        case '7': SEG_A = 0; SEG_B = 0; SEG_C = 0; break;
        case '8': SEG_A = 0; SEG_B = 0; SEG_C = 0; SEG_D = 0; SEG_E = 0; SEG_F = 0; SEG_G = 0; break;
        case '9': SEG_A = 0; SEG_B = 0; SEG_C = 0; SEG_D = 0; SEG_F = 0; SEG_G = 0; break;
        case 'A': SEG_A = 0; SEG_B = 0; SEG_C = 0; SEG_E = 0; SEG_F = 0; SEG_G = 0; break;
        case 'F': SEG_A = 0; SEG_E = 0; SEG_F = 0; SEG_G = 0; break;
        case 'I': SEG_B = 0; SEG_C = 0; break; // Same as '1'
        case 'L': SEG_D = 0; SEG_E = 0; SEG_F = 0; break;
        case 'M': SEG_A = 0; SEG_B = 0; SEG_C = 0; SEG_E = 0; SEG_F = 0; break; // Simplified
        case 'O': SEG_A = 0; SEG_B = 0; SEG_C = 0; SEG_D = 0; SEG_E = 0; SEG_F = 0; break; // Same as '0'
        case 'R': SEG_E = 0; SEG_G = 0; break; // Simplified
        case 'T': SEG_D = 0; SEG_E = 0; SEG_F = 0; SEG_G = 0; break;
        case '_': SEG_G = 0; break; // Underscore for "L_R", "L_M", "M_R"
        default: break;
    }
}

void main(void) {
    // Variables
    char display[3] = {'O', 'F', 'F'}; // Default "OFF"

    // Configure Port 0 (P0.0, P0.1, P0.3, P0.4) as push-pull outputs
    P0M1 &= ~0x1B;
    P0M2 |= 0x1B;

    // Configure Port 1 (P1.0, P1.1, P1.2, P1.3, P1.4, P1.5, P1.7) as push-pull outputs
    P1M1 &= ~0xBF;
    P1M2 |= 0xBF;

    // Configure P0.7 as ADC input
    AINDIDS |= (1 << 7);
    P0M1 |= (1 << 7);
    P0M2 &= ~(1 << 7);

    // Configure ADC
    ADCCON1 |= 0x01;
    ADCCON0 &= ~0x0F;
    ADCCON0 |= 0x02;

    // Initialize digits off
    DIGIT1 = 0;
    DIGIT2 = 0;
    DIGIT3 = 0;

    while (1) {
        unsigned int adc_value;
        unsigned char i;

        // Read ADC
        ADCCON0 |= (1 << 6);
        while (!(ADCCON0 & (1 << 7)));
        ADCCON0 &= ~(1 << 7);
        adc_value = (unsigned int)((ADCRH << 2) | (ADCRL >> 6));

        // Map ADC to button states
        if (adc_value >= 92 && adc_value <= 112) { // 0.5V (ALL)
            display[0] = 'A'; display[1] = 'L'; display[2] = 'L';
        } else if (adc_value >= 112 && adc_value <= 132) { // 0.6V (LEFT+MIDDLE)
            display[0] = 'L'; display[1] = '_'; display[2] = 'M';
        } else if (adc_value >= 133 && adc_value <= 153) { // 0.7V (LEFT+RIGHT)
            display[0] = 'L'; display[1] = '_'; display[2] = 'R';
        } else if (adc_value >= 153 && adc_value <= 173) { // 0.8V (LEFT)
            display[0] = 'L'; display[1] = 'F'; display[2] = 'T';
        } else if (adc_value >= 235 && adc_value <= 255) { // 1.2V (MIDDLE+RIGHT)
            display[0] = 'M'; display[1] = '_'; display[2] = 'R';
        } else if (adc_value >= 317 && adc_value <= 337) { // 1.6V (MIDDLE)
            display[0] = 'M'; display[1] = 'I'; display[2] = 'D';
        } else if (adc_value >= 501 && adc_value <= 521) { // 2.5V (RIGHT)
            display[0] = 'R'; display[1] = 'G'; display[2] = 'T';
        } else { // No buttons or other voltage
            display[0] = 'O'; display[1] = 'F'; display[2] = 'F';
        }

        // Multiplex display (5ms/digit, ~66Hz)
        for (i = 0; i < 5; i++) {
            set_segments(display[0]);
            DIGIT1 = 1; DIGIT2 = 0; DIGIT3 = 0;
            delay_ms(5);
            DIGIT1 = 0;

            set_segments(display[1]);
            DIGIT2 = 1; DIGIT1 = 0; DIGIT3 = 0;
            delay_ms(5);
            DIGIT2 = 0;

            set_segments(display[2]);
            DIGIT3 = 1; DIGIT1 = 0; DIGIT2 = 0;
            delay_ms(5);
            DIGIT3 = 0;
        }
    }
}
