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
#include <stdlib.h>
// ====================================================================
// Variables

// ====================================================================
// Function prototypes
void setup(void);
uint16_t leer_ADC6(void);
void enviar_string(char *s);
void enviar_char(char c);
void enviar_uint16_t(uint16_t valor);
char recibir_char(void);
void limpiar_terminal(void);
void actualizar_leds(uint8_t valor);
// ====================================================================
// Main Function
int main(void) {
	setup();
	
	char comando;
	uint8_t valor_leds = 0;
	enviar_string("\r\n=== Sistema ===\r\n");
	enviar_string("Comandos: ");
	enviar_string("\r\n> 1-ADC ");
	enviar_string("\r\n> 2-LEDs ");
	while(1) {
		// Refresco principal
		actualizar_leds(valor_leds);
		
		if (UCSR0A & (1 << RXC0)) {
			comando = recibir_char();
			
			if (comando == '1') {
				uint16_t valor_adc = leer_ADC6();
				limpiar_terminal();
				enviar_string("========================\r\n ");
				enviar_string("\r\nADC6 = ");
				enviar_uint16_t(valor_adc);
				enviar_string("\r\n\r\n========================\r\n ");
				enviar_string("\r\n=== Sistema ===\r\n");
				enviar_string("Comandos: ");
				enviar_string("\r\n> 1-ADC ");
				enviar_string("\r\n> 2-LEDs ");
			}
			else if (comando == '2') {
				uint16_t valor_temporal = 0;
				uint8_t entrada_valida = 0;
				uint8_t digito_count = 0;
				uint8_t leyendo = 0;
				
				enviar_string("Ingresa un valor de 3 digitos entre  0 y 255 ");
				leyendo = 1;
				
					    while(leyendo == 1) {
						    actualizar_leds(valor_leds);
						    
						    if (UCSR0A & (1 << RXC0)) {
							    char c = recibir_char();
							    
							    if (c >= '0' && c <= '9') {
								    enviar_char(c);  // eco
								    valor_temporal = valor_temporal * 10 + (c - '0');
								    digito_count++;
								    
								    if (digito_count == 3) {
									    if (valor_temporal <= 256) {
										    valor_leds = (uint8_t)valor_temporal;
													limpiar_terminal();
													enviar_string("========================\r\n ");
													enviar_string("\r\nOK: LEDs actualizados\r\n ");
													enviar_string("\r\n========================\r\n ");
													enviar_string("\r\n=== Sistema ===\r\n");
													enviar_string("Comandos: ");
													enviar_string("\r\n> 1-ADC ");
													enviar_string("\r\n> 2-LEDs ");
											
										    } else {
											limpiar_terminal();	
											enviar_string("========================\r\n ");
										    enviar_string("\r\nError: debe ser <= 255 ");
											enviar_string("\r\r========================\r\n ");
											enviar_string("\r\n=== Sistema ===\r\n");
											enviar_string("Comandos: ");
											enviar_string("\r\n> 1-ADC ");
											enviar_string("\r\n> 2-LEDs ");
									    }
									    leyendo = 0;  // salir del bucle
								    }
							    }
							    else if (c == '\r' || c == '\n') {
								    // ignorar enter
							    }
							    else {
									limpiar_terminal();
								    enviar_string("\r\nError: solo dígitos\r\n> ");
								    leyendo = 0;  // salir con error
							    }
						    }
					    }
				    }
				}
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
	
	cli();
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
//lee la terminal
char recibir_char(void) {
	while (!(UCSR0A & (1 << RXC0)));  // Esperar hasta recibir dato
	return UDR0;                       // Retornar el caracter recibido
}
//limpia el texto
void limpiar_terminal(void) {
	  // Enviar 50 saltos de línea para "limpiar la" pantalla
	  for (int i = 0; i < 50; i++) {
		  enviar_string("\r\n");
	}
}

// Función para actualizar LEDs multiplexados (llamar constantemente)
void actualizar_leds(uint8_t valor) {
	static uint8_t multiplexor = 0;
	uint8_t nibble_alto = (valor >> 4) & 0x0F;
	uint8_t nibble_bajo = valor & 0x0F;
	
	if (multiplexor == 0) {
		// Mostrar nibble alto (bits 4-7) activando PD7
		PORTD |= (1 << PORTD7);
		PORTD &= ~(1 << PORTD6);
		PORTB = (PORTB & 0xF0) | (nibble_alto & 0x0F);
		_delay_ms(10);
		PORTD &= ~(1 << PORTD6);
		PORTD &= ~(1 << PORTD7); 
		multiplexor = 1;
		} else {
		// Mostrar nibble bajo (bits 0-3) activando PD6
		PORTD |= (1 << PORTD6);
		PORTD &= ~(1 << PORTD7);
		PORTB = (PORTB & 0xF0) | (nibble_bajo & 0x0F);
		_delay_ms(10);
		PORTD &= ~(1 << PORTD6);
		PORTD &= ~(1 << PORTD7);
		multiplexor = 0;
		
	}
}

// ====================================================================