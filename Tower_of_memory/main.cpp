//programa Tower of memories 
//por said y alondra eso eso
//sistema navegador de biblioteca firs tupdate en git

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//caracteristicas de la ventana
static const int SW = 800;//valor de la parte horizontal osea el width :,3
static const int SH = 600;//valor de la parte vertical osea el height :,3
static const int SIDEBAR_W = 200;//ancho del sidebar 

//paleta de colores en rgb 
#define COLOR_BG        al_map_rgb(240, 240, 240)   // Fondo gris claro
#define COLOR_SIDEBAR   al_map_rgb(50, 50, 50)      // Sidebar gris oscuro
#define COLOR_BTN_NORMAL al_map_rgb(80, 80, 80)     // Botón normal
#define COLOR_BTN_ACTIVE al_map_rgb(120, 120, 120)  // Botón activo
#define COLOR_TEXT      al_map_rgb(255, 255, 255)   // Texto blanco
#define COLOR_BORDER    al_map_rgb(100, 100, 100)   // Borde


enum Section { SEC_PAGE1 = 0, SEC_PAGE2, SEC_PAGE3, SEC_PAGE4, SEC_PAGE5 };//enum para mostrar numero de pagina en el que estas
enum Estado { DISPONIBLE, PRESTADO };
static const char* BTN_LABELS[] = { "Pagina 1 xd", "Pagina 2", "Pagina 3", "Pagina 4", "Pagina 5" };//texto que se mostrara en los botones de la pagina
static Section current_page = SEC_PAGE1;

// ── Estructura de datos (similar a struct Amigo del codigo original) ─
#define TAM 50 // tamano maximo de cadenas, igual que en el original

struct Item {
    int    id;           // id unico generado automaticamente
    char   titulo[TAM];
    char   autor[TAM];
    int    año;
    char   genero[TAM];
    Estado estado;       // DISPONIBLE o PRESTADO, igual que el enum del original
};

// arreglo dinamico de items — se maneja con malloc/realloc/free como el original
static Item* items = NULL;
static int   num_items = 0;
#define ARCHIVO_BIN "tower_of_memory_books.bin" // archivo binario de persistencia

// ── Funciones de archivo y memoria (estilo del codigo original) ──────

// carga los items del archivo binario al iniciar — similar a fopen del original
void cargar_items() {
    FILE* arch;
    fopen_s(&arch, ARCHIVO_BIN, "rb"); // fopen_s: version segura para visual studio
    if (!arch) return;                 // si no existe el archivo todavia no pasa nada

    fread(&num_items, sizeof(int), 1, arch); // leer cuantos items hay guardados
    items = (Item*)malloc(num_items * sizeof(Item)); // asignar memoria con malloc
    fread(items, sizeof(Item), num_items, arch);     // leer todos los items de golpe

    fclose(arch); // cerrar archivo como en el original
}

// guarda el arreglo completo al archivo binario — similar a fprintf del original
void guardar_items() {
    FILE* arch;
    fopen_s(&arch, ARCHIVO_BIN, "wb"); // fopen_s modo escritura binaria
    if (!arch) return;

    fwrite(&num_items, sizeof(int), 1, arch);        // guardar cantidad de items
    fwrite(items, sizeof(Item), num_items, arch);    // guardar todos los items

    fclose(arch);
}

// agrega un nuevo item — similar a la funcion altas() del codigo original
// usa realloc para expandir el arreglo dinamico
void altas(const char* titulo, const char* autor, int año, const char* genero) {
    num_items++;
    items = (Item*)realloc(items, num_items * sizeof(Item)); // realloc como en el original

    Item* nuevo = &items[num_items - 1];
    nuevo->id = num_items;                 // id generado automaticamente
    strncpy_s(nuevo->titulo, TAM, titulo, TAM - 1);
    strncpy_s(nuevo->autor, TAM, autor, TAM - 1);
    nuevo->año = año;
    strncpy_s(nuevo->genero, TAM, genero, TAM - 1);
    nuevo->estado = DISPONIBLE;

    guardar_items(); // persistencia: actualizar archivo cada vez que se agrega
}

// elimina un item por id — actualiza arreglo y archivo (persistencia)
void bajas(int id) {
    int pos = -1;
    for (int i = 0; i < num_items; i++) {
        if (items[i].id == id) { pos = i; break; }
    }
    if (pos == -1) return; // no encontrado

    for (int i = pos; i < num_items - 1; i++) // recorrer para tapar el hueco
        items[i] = items[i + 1];

    num_items--;
    items = (Item*)realloc(items, num_items * sizeof(Item)); // ajustar memoria

    guardar_items(); // persistencia
}

// cambia estado entre DISPONIBLE y PRESTADO — persistencia inmediata
void cambiar_estado(int id) {
    for (int i = 0; i < num_items; i++) {
        if (items[i].id == id) {
            items[i].estado = (items[i].estado == DISPONIBLE) ? PRESTADO : DISPONIBLE;
            guardar_items();
            return;
        }
    }
}

// libera toda la memoria al cerrar — free como en el original
void liberar_items() {
    free(items);
    items = NULL;
    num_items = 0;
}

// ── Variables de estado del formulario de pagina1 (alta) ─────────────
static char input_titulo[TAM] = "";
static char input_autor[TAM] = "";
static char input_año[8] = "";
static char input_genero[TAM] = "";
static int  campo_activo = 0;     // 0=titulo 1=autor 2=año 3=genero
static char msg_status[64] = "";    // mensaje de feedback al usuario

// posiciones y medidas del formulario — constantes para no recalcular
static const float FORM_X = SIDEBAR_W + 20;
static const float FORM_FW = SW - SIDEBAR_W - 40; // ancho de los campos
static const float FORM_FH = 22;                  // alto de cada campo de texto
// y de inicio del primer campo (despues del titulo + linea + margen)
static const float FORM_Y0 = 71; // 20(titulo)+18(linea+margen)+16(etiqueta)+... se calcula abajo

// calcula la y de la caja i-esima del formulario
static float campo_y(int i) {
    // cada campo ocupa: etiqueta(16) + caja(22) + margen(10) = 48px
    return 20.0f + 25.0f + 10.0f + i * (16.0f + FORM_FH + 10.0f) + 16.0f;
}

// y del boton guardar (debajo del ultimo campo)
static float btn_guardar_y() {
    return campo_y(3) + FORM_FH + 14.0f;
}


static void draw_sidebar(ALLEGRO_FONT* font)
{
    // Fondo del sidebar
    al_draw_filled_rectangle(0, 0, SIDEBAR_W, SH, COLOR_SIDEBAR);//esta funcin dibuja un rectngulo relleno, los primeros 4 parametros 
    //son las coordenadas de las esquinas del rectngulo (x1, y1, x2, y2) y el ultimo parametro es el color del rectngulo eso 0,0 es la esquina 


    // Título
    al_draw_text(font, COLOR_TEXT, SIDEBAR_W / 2, 20, ALLEGRO_ALIGN_CENTRE, "Menu");//escribe menu en el sidebar te pide
    //la fuente,color,coordenada x, coordenada y, alineacion y el texto a escribir

    // Línea separadora
    al_draw_line(10, 50, SIDEBAR_W - 10, 50, COLOR_BORDER, 1);//dibuja una linea no tiene muhco chiste la veda

    // Botones de navegación
    float btn_y = 70;//coordenada y del primer boton
    float btn_h = 40;//cordenada y del segundo boton
    float btn_w = SIDEBAR_W - 20;//coordenada x del primer boton
    float btn_x = 10;//coordenada x del segundo boton

    for (int i = 0; i < 5; i++) {//ciclo para dibujar 5 botones
        bool active = (i == (int)current_page);

        // Fondo del botón
        ALLEGRO_COLOR btn_color = active ? COLOR_BTN_ACTIVE : COLOR_BTN_NORMAL;//si el boton es activo se pinta con un color mas claro y si no con un color mas oscuro
        al_draw_filled_rectangle(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, btn_color);//dibuja el rectangulo del boton con el color correspondiente
        //recibe igual tamano que el rectangulo del sidebar pero con un margen de 10 pixeles a cada lado
        al_draw_rectangle(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, COLOR_BORDER, 1);//dibuja el borde del boton con un color de borde

        // Texto del botón
        al_draw_text(font, COLOR_TEXT, btn_x + btn_w / 2, btn_y + btn_h / 2 - 6, ALLEGRO_ALIGN_CENTRE, BTN_LABELS[i]);//usa ek arreglo anteriior para poder saber 
        //que texto poner en cada butoncito


        if (active) {
            al_draw_text(font, COLOR_TEXT, btn_x + btn_w - 15, btn_y + btn_h / 2 - 6, 0, ">");//similar a el hover bueno focus sirve para cambiarle el diseno cuando estemos en su pagina
        }

        btn_y += btn_h + 25;//incrementa la coordenada y para el siguiente boton, dejando un espacio de 8  entre ellos
    }
}

static void draw_content_area()
{
    // Fondo blanco/gris claro para el área derecha
    al_draw_filled_rectangle(SIDEBAR_W, 0, SW, SH, COLOR_BG);//dibuja el fondo del area de contenido con un color gris 
    //empieza desde el borde del sidebar hasta el final de la ventana igual se le da width y height de la ventana para que ocupe todo el espacio disponible


    al_draw_line(SIDEBAR_W, 0, SIDEBAR_W, SH, COLOR_BORDER, 2);//dibuja un borde
}


// area disponible: x desde SIDEBAR_W(200) hasta SW(800), y desde 0 hasta SH(600)

// pagina1 — formulario de alta (agregar item)
// campos: titulo, autor, año, genero
// Tab avanza campo, clic selecciona campo, boton Guardar llama a altas()
static void dibujar_pagina1(ALLEGRO_FONT* font)
{
    float x = FORM_X;

    al_draw_text(font, al_map_rgb(0, 0, 0), x, 20, 0, "AGREGAR ITEM");
    al_draw_line(x, 45, x + FORM_FW, 45, COLOR_BORDER, 1);

    const char* etiquetas[] = { "Titulo:", "Autor:", "año:", "Genero:" };
    const char* valores[] = { input_titulo, input_autor, input_año, input_genero };

    for (int i = 0; i < 4; i++) {
        float cy = campo_y(i);

        al_draw_text(font, al_map_rgb(0, 0, 0), x, cy - 16, 0, etiquetas[i]);

        // caja resaltada si es el campo activo
        ALLEGRO_COLOR caja_color = (i == campo_activo)
            ? al_map_rgb(200, 220, 255) // azul claro = activo
            : al_map_rgb(255, 255, 255);// blanco = inactivo
        al_draw_filled_rectangle(x, cy, x + FORM_FW, cy + FORM_FH, caja_color);
        al_draw_rectangle(x, cy, x + FORM_FW, cy + FORM_FH, COLOR_BORDER, 1);
        al_draw_text(font, al_map_rgb(0, 0, 0), x + 4, cy + 4, 0, valores[i]);

        // cursor en el campo activo
        if (i == campo_activo) {
            int tw = al_get_text_width(font, valores[i]);
            al_draw_line(x + 5 + tw, cy + 3, x + 5 + tw, cy + FORM_FH - 3,
                al_map_rgb(0, 0, 0), 1);
        }
    }

    // boton Guardar
    float by = btn_guardar_y();
    al_draw_filled_rectangle(x, by, x + 90, by + 24, al_map_rgb(60, 130, 60));
    al_draw_rectangle(x, by, x + 90, by + 24, COLOR_BORDER, 1);
    al_draw_text(font, COLOR_TEXT, x + 45, by + 6, ALLEGRO_ALIGN_CENTRE, "Guardar");

    // mensaje de status debajo del boton
    if (msg_status[0] != '\0')
        al_draw_text(font, al_map_rgb(0, 120, 0), x, by + 32, 0, msg_status);
}

// pagina2 — mostrar todos los items (similar a "Mostrar" del menu original)
static void dibujar_pagina2(ALLEGRO_FONT* font)
{
    float x = FORM_X;
    float y = 20;

    al_draw_text(font, al_map_rgb(0, 0, 0), x, y, 0, "LISTA DE ITEMS");
    y += 20;
    al_draw_line(x, y, SW - 20, y, COLOR_BORDER, 1);
    y += 8;

    if (num_items == 0) {
        al_draw_text(font, al_map_rgb(150, 150, 150), x, y, 0, "No hay items registrados.");
        return;
    }

    // encabezados de columna
    al_draw_text(font, al_map_rgb(80, 80, 80), x, y, 0, "ID");
    al_draw_text(font, al_map_rgb(80, 80, 80), x + 30, y, 0, "Titulo");
    al_draw_text(font, al_map_rgb(80, 80, 80), x + 210, y, 0, "Autor");
    al_draw_text(font, al_map_rgb(80, 80, 80), x + 390, y, 0, "año");
    al_draw_text(font, al_map_rgb(80, 80, 80), x + 445, y, 0, "Estado");
    y += 16;
    al_draw_line(x, y, SW - 20, y, COLOR_BORDER, 1);
    y += 4;

    for (int i = 0; i < num_items; i++) {
        if (y > SH - 20) break; // no salir de pantalla

        // filas alternadas para mejor lectura
        ALLEGRO_COLOR fila = (i % 2 == 0) ? al_map_rgb(248, 248, 248) : al_map_rgb(232, 232, 232);
        al_draw_filled_rectangle(x - 2, y - 2, SW - 20, y + 14, fila);

        char id_str[8];   snprintf(id_str, 8, "%d", items[i].id);
        char año_str[8]; snprintf(año_str, 8, "%d", items[i].año);

        al_draw_text(font, al_map_rgb(0, 0, 0), x, y, 0, id_str);
        al_draw_text(font, al_map_rgb(0, 0, 0), x + 30, y, 0, items[i].titulo);
        al_draw_text(font, al_map_rgb(0, 0, 0), x + 210, y, 0, items[i].autor);
        al_draw_text(font, al_map_rgb(0, 0, 0), x + 390, y, 0, año_str);

        ALLEGRO_COLOR c_estado = (items[i].estado == DISPONIBLE)
            ? al_map_rgb(0, 130, 0) : al_map_rgb(180, 40, 40);
        al_draw_text(font, c_estado, x + 445, y, 0,
            items[i].estado == DISPONIBLE ? "DISPONIBLE" : "PRESTADO");
        y += 18;
    }
}

static void dibujar_pagina3(ALLEGRO_FONT* font)
{


}

static void dibujar_pagina4(ALLEGRO_FONT* font)
{


}

static void dibujar_pagina5(ALLEGRO_FONT* font)
{
    // TODO: tu contenido de pagina 5

}


int main()
{
    // Inicialización
    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    //inicializar todos los addons a lo menso yujuu

    cargar_items(); // cargar datos del archivo binario al iniciar con malloc

    // Crear ventana
    al_set_new_display_flags(ALLEGRO_WINDOWED);
    ALLEGRO_DISPLAY* display = al_create_display(SW, SH);//crea la ventana con el width y height definidos de hasta arriba 
    al_set_window_title(display, "Tower of Memory");//le pone un titulo a la ventana

    // Cola de eventos y timer
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();//crea una cola de eventos 
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);//crea un timer que se dispara 60 veces por segundo para controlar el redraw de la pantalla

    al_register_event_source(queue, al_get_display_event_source(display));//registra la fuente de eventos del display para poder detectar cuando se cierra la ventana
    al_register_event_source(queue, al_get_keyboard_event_source());//registra la fuente de eventos del teclado para poder detectar cuando se presionan teclas
    al_register_event_source(queue, al_get_mouse_event_source());//registra la fuente de eventos del mouse para poder detectar cuando se hacen clics o movimientos del mouse
    al_register_event_source(queue, al_get_timer_event_source(timer));//registra la fuente de eventos del timer para poder detectar cuando se dispara el timer 
    //y así controlar el redraw de la pantalla


    ALLEGRO_FONT* font = al_create_builtin_font();//crea una fuente de texto incorporada para poder dibujar texto igual esto se va a cambiar por algo asm gotico despues


    bool redraw = true;
    al_start_timer(timer);
    while (true)
    {
        ALLEGRO_EVENT ev;//variable para almacenar el evento que se va a procesar
        al_wait_for_event(queue, &ev);//espera a que ocurra un evento y lo almacena en la variable ev

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) break;//si el evento es cerrar la ventana, se sale del bucle principal y termina el programa

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            redraw = true;
        }

        // Clic del mouse para cambiar de página
        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1)//usa funcion de mouse para detectar cuando se hace clic con el boton izquierdo del mouse
        {
            float mx = ev.mouse.x;
            float my = ev.mouse.y;

            // Solo si el clic está dentro del sidebar
            if (mx <= SIDEBAR_W) {
                float btn_y = 70;
                float btn_h = 40;
                float btn_w = SIDEBAR_W - 20;
                float btn_x = 10;

                for (int i = 0; i < 5; i++) {
                    if (mx >= btn_x && mx <= btn_x + btn_w && my >= btn_y && my <= btn_y + btn_h) {
                        current_page = (Section)i;
                        break;
                    }
                    btn_y += btn_h + 25;
                }
            }

            // clic en pagina1 — seleccionar campo o presionar boton guardar
            if (current_page == SEC_PAGE1 && mx > SIDEBAR_W) {

                // detectar en cual caja hizo clic para activar ese campo
                for (int i = 0; i < 4; i++) {
                    float cy = campo_y(i);
                    if (mx >= FORM_X && mx <= FORM_X + FORM_FW && my >= cy && my <= cy + FORM_FH) {
                        campo_activo = i;
                        break;
                    }
                }

                // detectar clic en boton guardar
                float by = btn_guardar_y();
                if (mx >= FORM_X && mx <= FORM_X + 90 && my >= by && my <= by + 24) {
                    if (input_titulo[0] != '\0' && input_autor[0] != '\0' && input_genero[0] != '\0') {
                        altas(input_titulo, input_autor, atoi(input_año), input_genero);
                        // limpiar campos tras guardar
                        input_titulo[0] = '\0';
                        input_autor[0] = '\0';
                        input_año[0] = '\0';
                        input_genero[0] = '\0';
                        campo_activo = 0;
                        snprintf(msg_status, 64, "Guardado! Total items: %d", num_items);
                    }
                    else {
                        snprintf(msg_status, 64, "Completa todos los campos.");
                    }
                }
            }
        }

        // Salir con ESC
        if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) break;//obtenemos el valor o numero de la tecla key y si se umple rompemos

            // captura de texto en los campos de pagina1
            if (current_page == SEC_PAGE1) {
                char* campos[] = { input_titulo, input_autor, input_año, input_genero };
                int   tams[] = { TAM, TAM, 7, TAM };
                char* campo = campos[campo_activo];
                int   tam = tams[campo_activo];
                int   len = (int)strlen(campo);

                if (ev.keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
                    if (len > 0) campo[len - 1] = '\0'; // borrar ultimo caracter
                }
                else if (ev.keyboard.keycode == ALLEGRO_KEY_TAB) {
                    campo_activo = (campo_activo + 1) % 4; // avanzar campo con Tab
                }
                else if (ev.keyboard.unichar >= 32 && ev.keyboard.unichar < 127) {
                    if (len < tam - 1) {
                        campo[len] = (char)ev.keyboard.unichar;
                        campo[len + 1] = '\0'; // agregar caracter al campo activo
                    }
                }
            }
        }

        // Dibujo
        if (redraw && al_is_event_queue_empty(queue))
        {
            redraw = false;

            // Limpiar pantalla
            al_clear_to_color(COLOR_BG);

            // Dibujar sidebar y área de contenido
            draw_sidebar(font);
            draw_content_area();

            // Llama a la funcion de la pagina activa segun current_page
            switch (current_page)
            {
            case SEC_PAGE1: dibujar_pagina1(font); break;
            case SEC_PAGE2: dibujar_pagina2(font); break;
            case SEC_PAGE3: dibujar_pagina3(font); break;
            case SEC_PAGE4: dibujar_pagina4(font); break;
            case SEC_PAGE5: dibujar_pagina5(font); break;
            }

            al_flip_display();
        }
    }

    // Limpieza
    liberar_items(); // liberar toda la memoria dinamica con free al cerrar
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    //destrozamos todo basicamente osea cortar bla bla bla

    return 0;//regresamos el poderoso 0
}