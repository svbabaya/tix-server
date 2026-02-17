#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

#define PORT 8086

// Колбэк для обработки входящих запросов
static enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection,
                          const char *url, const char *method,
                          const char *version, const char *upload_data,
                          size_t *upload_data_size, void **con_cls) {
    (void)cls;
    (void)url;
    (void)method;
    (void)version;
    (void)upload_data;
    (void)upload_data_size;
    (void)con_cls;
    
    const char *page = "{\"status\": \"ok\", \"message\": \"TiXerver is online\"}";
    struct MHD_Response *response;
    enum MHD_Result ret;

    // Создаем HTTP-ответ из строки
    response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    
    // Устанавливаем заголовок Content-Type для текстового/JSON API
    MHD_add_response_header(response, "Content-Type", "application/json");

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main() {
    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, 
                                PORT, NULL, NULL,
                                &answer_to_connection, NULL,
                                MHD_OPTION_THREAD_POOL_SIZE, (unsigned int)4,
                                MHD_OPTION_END);


    if (NULL == daemon) return 1;

    printf("Server running on port %d. Press enter to stop.\n", PORT);
    getchar(); 

    MHD_stop_daemon(daemon);
    return 0;
}
