/*
 * PostLab4
 *
 * Author: Sebastian Da Silva 
 */
// ====================================================================
// Libraries
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
// ====================================================================
// Variables

// ====================================================================
// Function prototypes
void setup(void);
uint16_t leer_ADC6(void);
void enviar_string(char *s);
void enviar_char(char c);
void enviar_uint16_t(uint16_t valor);
// ====================================================================
// Main Function
int main(void)
{
	setup();
	enviar_string("Sistema iniciado\r\n");
    enviar_string("Valor ADC6:\r\n");
	while(1)
	{
	     uint16_t valor_adc = leer_ADC6();  // Leer ADC6
        
        enviar_uint16_t(valor_adc);        // Enviar valor numérico
        enviar_string("\r\n");              // Salto de línea
        
        _delay_ms(500);  // Enviar cada 500ms
	}
}
// ====================================================================
// NON-Interrupt subroutines

void setup(void) {
	// Configurar PB0 a PB3 como salidas para LEDs
	DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3);
	PORTB &= ~((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3));
	
	// Configurar ADC6 como entrada analógica
	ADMUX |= (1 << REFS0);                      // Referencia AVCC
	ADMUX |= (1 << MUX2) | (1 << MUX1);         // Seleccionar canal 6
	ADMUX &= ~((1 << MUX3) | (1 << MUX0));
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);  // Habilitar ADC, prescaler 64
	
	// Configurar PD6 y PD7 como salidas para multiplexar
	DDRD |= (1 << DDD6) | (1 << DDD7);
	PORTD &= ~((1 << PORTD6) | (1 << PORTD7));
// Inicializar USART a 9600 baudios (para 16MHz)
	// Configurar velocidad: 9600 baud @ 16MHz
    UBRR0H = 0;
    UBRR0L = 103;  // Formula: (F_CPU/(16*BAUD)) - 1 = (16000000/(16*9600)) - 1 = 103
    
    // Configurar formato: 8 bits, 1 stop bit, sin paridad
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    
    // Habilitar transmisor y receptor
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);	
}

	// Enviar un solo caracter
void enviar_char(char c) {
    while (!(UCSR0A & (1 << UDRE0)));  // Esperar buffer vacío
    UDR0 = c;
}
// Enviar una cadena de texto
void enviar_string(char *s) {
    while (*s) {
        enviar_char(*s++);
    }
}

// Enviar un número de 16 bits (0-65535) como texto
void enviar_uint16_t(uint16_t valor) {
    char buffer[6];  //65535 + null
    int i = 0;
    
    // Manejar caso especial 0
    if (valor == 0) {
        enviar_char('0');
        return;
    }
    
    // Convertir número a string (dígito por dígito)
    while (valor > 0) {
        buffer[i++] = '0' + (valor % 10);  // Último dígito
        valor /= 10;
    }
    
    // Enviar en orden inverso (dígitos correctos)
    while (i > 0) {
        enviar_char(buffer[--i]);
    }
}

// Leer ADC6
uint16_t leer_ADC6(void)
{
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}
// ====================================================================