#include "MS51_16K.H"

// Define segment pins (SEG_DP moved to P0.2)
sbit SEG_A = P0^5;  // Segment A (P0.5)
sbit SEG_B = P0^7;  // Segment B (P0.7)
sbit SEG_C = P0^1;  // Segment C (P0.1)
sbit SEG_D = P0^3;  // Segment D (P0.3)
sbit SEG_E = P0^4;  // Segment E (P0.4)
sbit SEG_F = P0^6;  // Segment F (P0.6)
sbit SEG_G = P0^0;  // Segment G (P0.0)
sbit SEG_DP = P0^2; // Decimal Point (P0.2)

// Define digit common pins (four digits, D5 dropped)
sbit DIGIT1 = P1^2; // Common Anode 1 (P1.2)
sbit DIGIT2 = P1^0; // Common Anode 2 (P1.0)
sbit DIGIT3 = P3^0; // Common Anode 3 (P3.0)
sbit DIGIT4 = P1^1; // Common Anode 4 (P1.1)

// Delay function (tuned for ~16MHz system clock)
void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 160; j++); // Approx. 0.48ms per iteration
}

// Function to set segments based on a character
void set_segments(char c, bit dp) {
    // Turn off all segments
    SEG_A = 1;
    SEG_B = 1;
    SEG_C = 1;
    SEG_D = 1;
    SEG_E = 1;
    SEG_F = 1;
    SEG_G = 1;
    SEG_DP = !dp; // DP on if dp=1

    // Validate input character
    if (c < '0' || c > '9') {
        c = '0'; // Default to '0' for invalid input
    }

    // Set segment patterns
    switch (c) {
        case '0':
            SEG_A = 0; SEG_B = 0; SEG_C = 0;
            SEG_D = 0; SEG_E = 0; SEG_F = 0;
            break;
        case '1':
            SEG_B = 0; SEG_C = 0;
            break;
        case '2':
            SEG_A = 0; SEG_B = 0; SEG_D = 0;
            SEG_E = 0; SEG_G = 0;
            break;
        case '3':
            SEG_A = 0; SEG_B = 0; SEG_C = 0;
            SEG_D = 0; SEG_G = 0;
            break;
        case '4':
            SEG_B = 0; SEG_C = 0; SEG_F = 0;
            SEG_G = 0;
            break;
        case '5':
            SEG_A = 0; SEG_C = 0; SEG_D = 0;
            SEG_F = 0; SEG_G = 0;
            break;
        case '6':
            SEG_A = 0; SEG_C = 0; SEG_D = 0;
            SEG_E = 0; SEG_F = 0; SEG_G = 0;
            break;
        case '7':
            SEG_A = 0; SEG_B = 0; SEG_C = 0;
            break;
        case '8':
            SEG_A = 0; SEG_B = 0; SEG_C = 0;
            SEG_D = 0; SEG_E = 0; SEG_F = 0; SEG_G = 0;
            break;
        case '9':
            SEG_A = 0; SEG_B = 0; SEG_C = 0;
            SEG_D = 0; SEG_F = 0; SEG_G = 0;
            break;
        default:
            break;
    }
}

void main(void) {
    // Variables
    char display[4] = {'0', '0', '0', '0'}; // Initialize to "0.000"

    // Configure Port 0 (P0.0, P0.1, P0.2, P0.3, P0.4, P0.5, P0.6, P0.7) as push-pull outputs
    P0M1 &= ~0xFF; // Clear mode bits for P0.0–P0.7
    P0M2 |= 0xFF;  // Set push-pull for P0.0–P0.7

    // Configure Port 1 (P1.0, P1.1, P1.2) as push-pull outputs
    P1M1 &= ~0x07; // Clear mode bits for P1.0, P1.1, P1.2
    P1M2 |= 0x07;  // Set push-pull for P1.0, P1.1, P1.2

    // Configure Port 3 (P3.0) as push-pull output
    P3M1 &= ~0x01;
    P3M2 |= 0x01;

    // Configure P1.7 as ADC input (configured as CH0)
    AINDIDS |= (1 << 7); // Enable AIN7 (P1.7, treated as AIN0 for CH0)
    P1M1 |= (1 << 7);    // Set P1.7 as input
    P1M2 &= ~(1 << 7);

    // Configure ADC for P1.7 (CH0, as requested)
    ADCCON1 |= 0x01;       // Enable ADC
    ADCCON0 &= ~0x0F;      // Clear channel bits
    ADCCON0 |= 0x00;       // Select channel 0 (P1.7, non-standard)

    // Initialize digits off
    DIGIT1 = 0;
    DIGIT2 = 0;
    DIGIT3 = 0;
    DIGIT4 = 0;

    while (1) {
        unsigned int adc_value;
        unsigned int volt;
        unsigned char i;

        // Read ADC
        ADCCON0 |= (1 << 6);  // Start conversion
        while (!(ADCCON0 & (1 << 7))); // Wait for completion
        ADCCON0 &= ~(1 << 7); // Clear ADC flag
        adc_value = (unsigned int)((ADCRH << 2) | (ADCRL >> 6));

        // Validate ADC value
        if (adc_value > 1023) {
            adc_value = 1023; // Cap at max 10-bit value
        }

        // Convert to volts (X.XXX format)
        volt = (unsigned long)adc_value * 5000 / 1023; // 0–5000 (0.000–5.000V)
        if (volt > 5000) {
            volt = 5000; // Cap at 5.000V
        }

        // Extract digits for X.XXX
        display[0] = '0' + (volt / 1000);        // Integer part (0–5)
        display[1] = '0' + ((volt % 1000) / 100); // First decimal (0–9)
        display[2] = '0' + ((volt % 100) / 10);  // Second decimal (0–9)
        display[3] = '0' + (volt % 10);          // Third decimal (0–9)

        // Validate display digits
        if (display[0] < '0' || display[0] > '5') {
            display[0] = '0'; // Ensure Digit 1 is 0–5
        }
        for (i = 1; i < 4; i++) {
            if (display[i] < '0' || display[i] > '9') {
                display[i] = '0';
            }
        }

        // Multiplex display (4ms/digit, ~62.5Hz)
        for (i = 0; i < 5; i++) {
            // Digit 1 (integer part, with DP)
            set_segments(display[0], 1);
            DIGIT1 = 1; DIGIT2 = 0; DIGIT3 = 0; DIGIT4 = 0;
            delay_ms(4);
            DIGIT1 = 0;

            // Digit 2 (first decimal)
            set_segments(display[1], 0);
            DIGIT2 = 1; DIGIT1 = 0; DIGIT3 = 0; DIGIT4 = 0;
            delay_ms(4);
            DIGIT2 = 0;

            // Digit 3 (second decimal)
            set_segments(display[2], 0);
            DIGIT3 = 1; DIGIT1 = 0; DIGIT2 = 0; DIGIT4 = 0;
            delay_ms(4);
            DIGIT3 = 0;

            // Digit 4 (third decimal)
            set_segments(display[3], 0);
            DIGIT4 = 1; DIGIT1 = 0; DIGIT2 = 0; DIGIT3 = 0;
            delay_ms(4);
            DIGIT4 = 0;
        }
    }
}
