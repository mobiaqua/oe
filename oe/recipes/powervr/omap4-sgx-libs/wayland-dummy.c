struct {
    void (*destroy)(void *a1, void *a2);
} wl_buffer_interface;
void wl_resource_destroy(void *a1) {}
unsigned int wl_client_add_resource(void *a1, void *a2) { return 0; }
void *wl_display_add_global(void *a1, void *a2, void *a3, void *a4) { return 0; }
void wl_resource_post_no_memory(void *a1) {}
void *wl_client_add_object(void *a1, void *a2, void *a3, unsigned int a4, void *a5) { return 0; }
void wl_resource_post_event(void *a1, unsigned int a2, ...) {}
