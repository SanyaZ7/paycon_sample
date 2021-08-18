#include <stdlib.h>
#include <gtk/gtk.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

using namespace std;
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <time.h>

char* open_text_file_libc(char *filename)
{
    struct stat buf;
    int fh, result; ///декскрипторы
    FILE *input = NULL;
    fh=open(filename, O_RDONLY); ///получение свойств файла
    result=fstat(fh, &buf);
    if (result !=0)
        {
            printf("Плох дескриптор файла\n");
            return 0;
        }
    else
    {
        printf("%s",filename);
        printf("Размер файла: %ld\n", buf.st_size);
        printf("Номер устройства: %lu\n", buf.st_dev);
        printf("Время модификации: %s", ctime(&buf.st_atime));
        input = fopen(filename, "r"); ///получение содержимого файла
        if (input == NULL)
        {
            printf("Error opening file");
            return 0;
        }
        char* str=(char*) malloc(buf.st_size);
        fread(str,sizeof(char),buf.st_size, input);
        fclose(input);
        return str;
    }
}

typedef struct{
GtkApplication *app;
GtkWidget *win;
GtkAdjustment *h_adj;
GtkAdjustment *v_adj;
GtkWidget *scrolledwindow;
GtkWidget *from_Api;
GtkWidget *from_file;
GtkWidget *box_for_2_buttons;
GtkWidget *viewport;
GtkWidget *grid;
char *text;
vector<int> disp;
vector<int>row;
vector<int>column;
int h=0;
bool h_enable=false;
}appdata;

gboolean wheel_cb(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
{
    appdata *data=(appdata*) user_data;
    printf("scroll_direction_y=%f\n",event->scroll.delta_y);
    printf("adj=%f\n", gtk_adjustment_get_value(data->v_adj));
    int h=0;
    gtk_widget_get_preferred_height(data->grid,0,&h);
                    printf("h1=%d\n", h);
                    GtkAllocation alloc;
            gtk_widget_get_allocation(data->grid,&alloc);
                    printf("h2=%d\n", alloc.height);
    return FALSE;
}

/*void
size_allocate_cb (GtkWidget    *widget,
               GdkRectangle *allocation,
               gpointer      user_data)
{
    appdata *data=(appdata*) user_data;
    data->h_enable=true;
    data->h=allocation->height;
    printf("data->h=%d\n",data->h);
}*/

void from_file_clicked_cb (GtkButton *button, gpointer user_data)
{
    appdata *data=(appdata*) user_data;

    //clock_t start = clock();
    /*data->dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    data->spinner=gtk_spinner_new();
    g_object_set(G_OBJECT(data->dialog),"width-request", 128);
    g_object_set(G_OBJECT(data->dialog),"height-request", 128);
    gtk_container_add(GTK_CONTAINER(data->dialog),data->spinner);
    gtk_spinner_start(GTK_SPINNER(data->spinner));
    gtk_window_set_modal(GTK_WINDOW(data->dialog), TRUE);
    gtk_widget_show_all(data->dialog);*/

    ///g_object_ref () чтобы data->box_for_2_buttons не удалилось
    gtk_container_remove(GTK_CONTAINER(data->win), data->box_for_2_buttons);
    data->h_adj=gtk_adjustment_new(0,0,100,1,10,10);
    data->v_adj=gtk_adjustment_new(0,0,100,1,10,10);
    data->scrolledwindow=gtk_scrolled_window_new(data->h_adj, data->v_adj);
    data->viewport=gtk_viewport_new(data->h_adj, data->v_adj);
    data->grid=gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(data->viewport),data->grid);
    gtk_container_add(GTK_CONTAINER(data->scrolledwindow), data->viewport);
    gtk_container_add(GTK_CONTAINER(data->win), data->scrolledwindow);
    gtk_widget_show_all(data->win);
    gtk_widget_set_events(data->viewport,GDK_STRUCTURE_MASK|GDK_SCROLL_MASK);
    g_signal_connect(GTK_WIDGET(data->viewport), "scroll-event", G_CALLBACK(wheel_cb), data);
    //g_signal_connect(GTK_WIDGET(data->grid), "size-allocate", G_CALLBACK(size_allocate_cb), data);
    data->text=open_text_file_libc("База для тестового2.csv");
    bool quote_open=false;
    data->disp.emplace_back(0);
    printf("strlen=%d\n",strlen(data->text));
    int size=(int)strlen(data->text);
    int prev=0;  ///положение в векторе disp на начало предыдущей строки
    int row_count=0; ///номер строки в GtkGrid
    for(int i=0;i<=size;++i)
    {
        if(data->text[i]=='\"') {quote_open=!quote_open; continue;}
        if(data->text[i]==','&&quote_open==true) {continue;}
        if(data->text[i]==','&&quote_open==false)
        {
            ///не включать те случаи, когда запятая стоит в конце строки
            if(data->text[i+1]!='\n'&&data->text[i+2]!='\n')
            data->disp.emplace_back(i+1);
            data->text[i]=0;
            continue;
        }
       // printf("text[i]=3\n");
        if(data->text[i]=='\n')
        {
            data->text[i]=0;
            int n=data->disp.size();
            //printf("disp.size=%d\n",n);
            for(int j=prev;j<n;++j)
            {
                data->row.emplace_back(row_count);
                data->column.emplace_back(j-prev);
            }
            prev=n;++row_count;
            data->disp.emplace_back(i+1);
            continue;
        }
        if(data->text[i]==0)
        {
            printf("to_end\n");
            int n=data->disp.size();
            for(int j=prev;j<n;++j)
            {
                GtkTextTagTable *tagtable=gtk_text_tag_table_new();
                GtkTextBuffer *buffer=gtk_text_buffer_new(tagtable);
                gtk_text_buffer_set_text(buffer, (char*) data->text+data->disp.at(j),-1);
                GtkWidget * textview=gtk_text_view_new_with_buffer(buffer);
                gtk_grid_attach (GTK_GRID(data->grid), textview,j-prev,row_count,1,1);
                gtk_widget_show_all(data->grid);
                data->row.emplace_back(row_count);
                data->column.emplace_back(j-prev);
            }
            continue;
        }

    }
    clock_t start = clock();
    int h=0;
                int j=0;
                while(j<200)
                {
                GtkTextTagTable *tagtable=gtk_text_tag_table_new();
                GtkTextBuffer *buffer=gtk_text_buffer_new(tagtable);
                gtk_text_buffer_set_text(buffer, (char*) data->text+data->disp.at(j),-1);
                GtkWidget *textview=gtk_text_view_new_with_buffer(buffer);
                gtk_grid_attach (GTK_GRID(data->grid), textview,data->column[j],data->row[j],1,1);
                ++j;
                data->h_enable=false;
            }
            gtk_widget_show_all(data->grid);
            //printf("data->h=%d\n", data->h);

    /*for(int i=0;i<data->disp.size();++i)
    {
        printf("disp=%d|row=%d+column=%d|text=%s\n", data->disp[i], data->row[i], data->column[i], data->text+data->disp[i]);
    }*/
    //free(text);
    clock_t end = clock();
    printf("time=%lf\n",(end - start)/((double)CLOCKS_PER_SEC));
}

void application_activate(GtkApplication *application, gpointer user_data)
{
	appdata *data=(appdata*) user_data;
    printf("application activate\n");
	data->win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(data->win,800,600);
	gtk_window_set_decorated(GTK_WINDOW(data->win),TRUE);
	gtk_application_add_window(data->app,GTK_WINDOW(data->win));
	data->from_Api=gtk_button_new_with_label("Загрузить из API");
	data->from_file=gtk_button_new_with_label("Загрузить из файла");
	data->box_for_2_buttons=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_container_add(GTK_CONTAINER(data->win),data->box_for_2_buttons);
    g_object_set(G_OBJECT(data->box_for_2_buttons),"baseline-position", GTK_BASELINE_POSITION_CENTER);
    gtk_widget_set_halign(data->from_Api,GTK_ALIGN_CENTER);
    gtk_widget_set_valign(data->from_Api,GTK_ALIGN_CENTER);
    gtk_widget_set_halign(data->from_file,GTK_ALIGN_CENTER);
    gtk_widget_set_valign(data->from_file,GTK_ALIGN_CENTER);
    gtk_box_pack_start (GTK_BOX(data->box_for_2_buttons), data->from_file, TRUE, TRUE,0);
    gtk_box_pack_start (GTK_BOX(data->box_for_2_buttons), data->from_Api, TRUE, TRUE,0);

	g_signal_connect(data->from_file, "clicked", G_CALLBACK(from_file_clicked_cb), data);

    gtk_widget_show_all(data->win);
}

void application_shutdown(GtkApplication *application, gpointer user_data)
{
    /*appdata *data=user_data;
    g_ptr_array_free(data->all_svg,TRUE);
    data->pmenu=g_object_ref_sink(data->pmenu);
    g_clear_object(&(data->pmenu));
    data->fixed=g_object_ref_sink(data->fixed);
    gtk_widget_destroy(data->fixed);
    //g_clear_object(&(data->fixed));
    //g_clear_object(&(data->win));
    gtk_widget_destroy(data->win);
    g_clear_object(&(data->builder));*/
}

int main (int argc, char *argv[])
{
	appdata data;
	gtk_init (&argc, &argv);
	gint res;
	data.app = gtk_application_new("gtk3.ru", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(data.app, "activate", G_CALLBACK(application_activate), &data);
	g_signal_connect(data.app, "shutdown", G_CALLBACK(application_shutdown), &data);
	res = g_application_run(G_APPLICATION(data.app), 0, NULL);
return 0;
}
