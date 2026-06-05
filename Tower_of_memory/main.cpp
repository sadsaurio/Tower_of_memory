//programa Tower of memories 
//por said y alondra eso eso
//sistema navegador de biblioteca firs tupdate en git

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

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

static const char* BTN_LABELS[] = { "Pagina 1 xd", "Pagina 2", "Pagina 3", "Pagina 4", "Pagina 5" };//texto que se mostrara en los botones de la pagina
static Section current_page = SEC_PAGE1;//variable global para saber en que pagina estas y inicializarla en la 1 pq si


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

		btn_y += btn_h + 8;//incrementa la coordenada y para el siguiente boton, dejando un espacio de 8  entre ellos
    }
}

static void draw_content_area()
{
    // Fondo blanco/gris claro para el área derecha
	al_draw_filled_rectangle(SIDEBAR_W, 0, SW, SH, COLOR_BG);//dibuja el fondo del area de contenido con un color gris 
	//empieza desde el borde del sidebar hasta el final de la ventana igual se le da width y height de la ventana para que ocupe todo el espacio disponible

   
    al_draw_line(SIDEBAR_W, 0, SIDEBAR_W, SH, COLOR_BORDER, 2);//dibuja un borde
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
                    btn_y += btn_h + 8;
                }
            }
        }

        // Salir con ESC
        if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) break;//obtenemos el valor o numero de la tecla key y si se umple rompemos
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

            al_flip_display();
        }
    }

    // Limpieza
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    //destrozamos todo basicamente osea cortar bla bla bla

    return 0;//regresamos el poderoso 0
}