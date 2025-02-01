#include <stdio.h>                  // Biblioteca padrão de entrada e saída
#include "pico/stdlib.h"            // Biblioteca padrão para o Raspberry Pi Pico
#include "hardware/pio.h"           // Biblioteca para manipulação de periféricos PIO
#include "hardware/clocks.h"        // Biblioteca para controle de relógios do hardware
#include "ws2818b.pio.h"            // Programa para controle da matriz de LEDs WS2812

#define LED_RGB 13                  // Pino GPIO conectado ao LED RGB (vermelho)
#define LED_COUNT 25                // Número de LEDs na matriz
#define LED_PIN 7                   // Pino GPIO conectado a matriz de LEDs

static volatile uint32_t last_time = 0;     // Armazena o tempo do último evento (em microssegundos)
static int32_t set_button = 0;              // Controlador de seleção das animações
static volatile bool set_anime = 0;         // Controlador de seleção do tipo de animações
volatile int combination_index = 0;         // Índice da combinação atual

int led_count = 0;                      // Contador do led

const uint button_A = 5;             // GPIO do botão A
const uint button_B = 6;             // GPIO do botão B
const uint button_joy = 22;          // GPIO do botão joystick

struct pixel_t { 
    uint32_t G, R, B;                // Componentes de cor: Verde, Vermelho e Azul
};


typedef struct pixel_t pixel_t;     // Alias para a estrutura pixel_t
typedef pixel_t npLED_t;            // Alias para facilitar o uso no contexto de LEDs

npLED_t leds[LED_COUNT];            // Array para armazenar o estado de cada LED
PIO np_pio;                         // Variável para referenciar a instância PIO usada
uint sm;                            // Variável para armazenar o número do state machine usado

// Função para obter o índice de um LED na matriz
int getIndex(int x, int y) {
    // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
    // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
    if (y % 2 == 0) {
        return 24-(y * 5 + x);      // Linha par (esquerda para direita).
    } else {
        return 24-(y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
    }
}

// Função para inicializar o PIO para controle dos LEDs
void npInit(uint pin) 
{
    uint offset = pio_add_program(pio0, &ws2818b_program);      // Carregar o programa PIO
    np_pio = pio0;                                              // Usar o primeiro bloco PIO

    sm = pio_claim_unused_sm(np_pio, false);                    // Tentar usar uma state machine do pio0
    if (sm < 0)                                                 // Se não houver disponível no pio0
    {
        np_pio = pio1;                                          // Mudar para o pio1
        sm = pio_claim_unused_sm(np_pio, true);                 // Usar uma state machine do pio1
    }

    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);    // Inicializar state machine para LEDs

    for (uint i = 0; i < LED_COUNT; ++i)                        // Inicializar todos os LEDs como apagados
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Função para definir a cor de um LED específico
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) 
{
    leds[index].R = r;                                    // Definir componente vermelho
    leds[index].G = g;                                    // Definir componente verde
    leds[index].B = b;                                    // Definir componente azul
}

// Função para limpar (apagar) todos os LEDs
void npClear() 
{
    for (uint i = 0; i < LED_COUNT; ++i)                  // Iterar sobre todos os LEDs
        npSetLED(i, 0, 0, 0);                             // Definir cor como preta (apagado)
}

// Função para atualizar os LEDs no hardware
void npWrite() 
{
    for (uint i = 0; i < LED_COUNT; ++i)                      // Iterar sobre todos os LEDs
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G<<24);       // Enviar componente verde
        pio_sm_put_blocking(np_pio, sm, leds[i].R<<24);       // Enviar componente vermelho
        pio_sm_put_blocking(np_pio, sm, leds[i].B<<24);       // Enviar componente azul
    }
}

// Função para imprimir um frame na matriz de LEDs de maneira padronizada e sem dificuldades
void print_frame(int frame[5][5][3])
{
    for(int linha = 0; linha < 5; linha++){
        for(int coluna = 0; coluna < 5; coluna++){
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, frame[coluna][linha][0], frame[coluna][linha][1], frame[coluna][linha][2]);
        }
    }
    npWrite();
    
}

// Função para imprimir a animação dos números árabicos
void animation_number_ara(int number){

    //Lista dos numeros arabicos
    int number_1[5][5][3] = { //1
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0},}
                };
    int number_2[5][5][3] = { //2
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_3[5][5][3] = { //3
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_4[5][5][3] = { //4
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_5[5][5][3] = { //5
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_6[5][5][3] = { //6
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_7[5][5][3] = { //7
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_8[5][5][3] = { //8
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_9[5][5][3] = { //9
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_0[5][5][3] = { //0
                {{0, 0, 0}, {55, 0, 0}, {255, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {55, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };

    switch (number) {
        case 0: print_frame(number_0); break;
        case 1: print_frame(number_1); break;
        case 2: print_frame(number_2); break;
        case 3: print_frame(number_3); break;
        case 4: print_frame(number_4); break;
        case 5: print_frame(number_5); break;
        case 6: print_frame(number_6); break;
        case 7: print_frame(number_7); break;
        case 8: print_frame(number_8); break;
        case 9: print_frame(number_9); break;
    }          

}

// Função para imprimir a animação dos números poligonais
void animation_number_pol(int number){

    //Lista dos numeros em poligonos
    int number_1[5][5][3] = { //ponto
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},}
                };
    int number_2[5][5][3] = { //linha
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},}
                };
    int number_3[5][5][3] = { //triangulo
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0},}
                };
    int number_4[5][5][3] = { //quadrado
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0},}
                };
    int number_5[5][5][3] = { //pentagono
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_6[5][5][3] = { //hexagono
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0},}
                };
    int number_7[5][5][3] = { //heptagono
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_8[5][5][3] = { //octagono
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_9[5][5][3] = { //enneagono
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{55, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {55, 0, 0}},
                {{0, 0, 0}, {55, 0, 0}, {0, 0, 0}, {55, 0, 0}, {0, 0, 0},}
                };
    int number_0[5][5][3] = { //nada
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
                {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},}
                };
    
    switch (number) {
        case 0: print_frame(number_0); break;
        case 1: print_frame(number_1); break;
        case 2: print_frame(number_2); break;
        case 3: print_frame(number_3); break;
        case 4: print_frame(number_4); break;
        case 5: print_frame(number_5); break;
        case 6: print_frame(number_6); break;
        case 7: print_frame(number_7); break;
        case 8: print_frame(number_8); break;
        case 9: print_frame(number_9); break;
    }         
};

// Função para lidar com a interrupção dos botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    
    uint32_t current_time = to_us_since_boot(get_absolute_time());      // Obter o tempo atual em microssegundos

    if(current_time - last_time > 300000) {                             // Ignorar eventos muito próximos
       
        last_time = current_time;
        
        if (gpio == button_B) {                                         // Avança para a próxima combinação
            set_button++;
            if(set_button > 9){
                set_button = 9;
            } 
        } 
        if (gpio == button_A) {                                         // Retrocede para a combinação anterior
            set_button--;
            if(set_button < 0){
                set_button = 0;
            }
        }
        if (gpio == button_joy) {
            set_anime = !set_anime;                                     // Alterna entre os tipos de animações
        }
    
    }

}

// Função para executar a lógica principal do programa
void loop_playground() {

    gpio_put(LED_RGB, 1);                             // Acender o LED RGB
    sleep_ms(100);                                    // Aguardar 100ms
    gpio_put(LED_RGB, 0);                             // Apagar o LED RGB
    sleep_ms(100);                                    // Aguardar 100ms
    led_count++;                                      // Incrementar o contador de acionamentos do LED RGB
    switch (set_anime) {
        case 0: animation_number_ara(set_button); break;
        case 1: animation_number_pol(set_button); break;
    }
    
}

int main()
{
    //Inicialização dos periféricos
    stdio_init_all();                                     // Inicializar a comunicação serial
    gpio_init(LED_RGB);                                   // Inicializar o pino do LED RGB
    gpio_set_dir(LED_RGB, GPIO_OUT);                      // Configurar o pino do LED RGB como saída
    
    gpio_init(button_A);                                  // Inicializar o pino do botão A
    gpio_set_dir(button_A, GPIO_IN);                      // Configurar o pino do botão A como entrada
    gpio_pull_up(button_A);                               // Habilitar o pull-up interno do pino do botão A
    gpio_init(button_B);                                  // Inicializar o pino do botão B
    gpio_set_dir(button_B, GPIO_IN);                      // Configurar o pino do botão B como entrada
    gpio_pull_up(button_B);                               // Habilitar o pull-up interno do pino do botão B
    gpio_init(button_joy);                                // Inicializar o pino do botão joystick
    gpio_set_dir(button_joy, GPIO_IN);                    // Configurar o pino do botão joystick como entrada
    gpio_pull_up(button_joy);                             // Habilitar o pull-up interno do pino do botão joystick
    
    npInit(LED_PIN);                                      // Inicializar os LEDs
    npClear();                                            // Apagar todos os LEDs
    npWrite();                                            // Atualizar os LEDs no hardware
    
    //Configuração da interrupção do botão A
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilitar interrupção no botão A
    //Configuração da interrupção do botão B
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilitar interrupção no botão B
    //Configuração da interrupção do botão joystick
    gpio_set_irq_enabled_with_callback(button_joy, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilitar interrupção no botão joystick

    while (true) {
        loop_playground();
        
        // Verificar a contagem de acionamentos do LED a cada segundo
        static uint32_t last_time = 0;
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        if (current_time - last_time >= 1000) {         // Verifica se passou 1 segundo
            printf("O LED acendeu %d vezes no último segundo.\n", led_count);
            led_count = 0;                              // Reseta a contagem
            last_time = current_time;
        }
    }
    
    return 0;
}
