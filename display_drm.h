enum connections{
    connected = 1,
    disconnected = 2,
    unknown = 3,
};

struct framebuffer{
    int fb_id;
    int height;
    int width;
    int size;
    char * addr;
};

struct display{
    int fd;
    int connector_id;
    uint64_t * ptr_connector_id;
    int encoder_id;
    int crtc_id;
    struct framebuffer * fbs[2];
    struct drm_mode_modeinfo mode;
};

int drm_ioctl(int fd, int cmd, void * data);
void drm_get_resources(int fd, struct drm_mode_card_res * res);
void drm_get_connector(int fd, int connector_id, struct drm_mode_get_connector * connector);
void drm_get_encoder(int fd, int encoder_id, struct drm_mode_get_encoder * enc);
void create_framebuffer(int fd, int width, int height, struct framebuffer * fb);
void drm_set_crtc(struct display * display);
void drm_page_flip(int fd, int fb_id, int crtc_id);
int open_dev();
void start_display(struct display * display);
void create_framebuffers(struct display * display);
void show_modes(int fd);
struct display * choose_mode(int fd, int height, int width);