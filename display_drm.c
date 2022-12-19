#include <stdio.h>
#include <fcntl.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>

#include "display_drm.h"

int drm_ioctl(int fd, int cmd, void * data){
    int ret = ioctl(fd, cmd, data);
    if(ret < 0){
        printf("drm ioctl error");
        exit(0);
    }
    return ret;
}

void drm_get_resources(int fd, struct drm_mode_card_res * res){

    memset(res, 0, sizeof(struct drm_mode_card_res));
	drm_ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, res);

	if(res->count_connectors){
		res->connector_id_ptr = (uint64_t)malloc(res->count_connectors * sizeof(int));
	}
	if(res->count_encoders){
		res->encoder_id_ptr = (uint64_t)malloc(res->count_encoders * sizeof(int));
	}
	if(res->count_crtcs){
		res->crtc_id_ptr = (uint64_t)malloc(res->count_crtcs * sizeof(int));
	}
	
    drm_ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, res);

}

void drm_get_connector(int fd, int connector_id, struct drm_mode_get_connector * connector){

    memset(connector, 0, sizeof(struct drm_mode_get_connector));
    connector->connector_id = connector_id;

    drm_ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, connector);

    if(connector->count_modes){
        connector->modes_ptr = (uint64_t)malloc(connector->count_modes * sizeof(struct drm_mode_modeinfo));
    }
    if(connector->count_props){
        connector->props_ptr = (uint64_t)malloc(connector->count_props * sizeof(uint32_t));
        connector->prop_values_ptr = (uint64_t)malloc(connector->count_props * sizeof(uint64_t));
    }
    if(connector->count_encoders){
        connector->encoders_ptr = (uint64_t)malloc(connector->count_encoders * sizeof(int)); 
    }

    drm_ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, connector);

}

void drm_get_encoder(int fd, int encoder_id, struct drm_mode_get_encoder * enc){

    memset(enc, 0, sizeof(struct drm_mode_get_encoder));
    enc->encoder_id = encoder_id;
    drm_ioctl(fd, DRM_IOCTL_MODE_GETENCODER, enc);

}

void create_framebuffer(int fd, int width, int height, struct framebuffer * fb){

    struct drm_mode_create_dumb dumb;
    struct drm_mode_map_dumb mdumb;
    struct drm_mode_fb_cmd fbcmd;

    memset(&dumb, 0, sizeof(dumb));
    memset(&fbcmd, 0, sizeof(fbcmd));
    memset(&mdumb, 0, sizeof(mdumb));

    dumb.width = width;
    dumb.height = height;
    dumb.bpp = 32;
    drm_ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &dumb);

    fbcmd.width = dumb.width;
    fbcmd.height = dumb.height;
    fbcmd.depth = 24;
    fbcmd.bpp = dumb.bpp;
    fbcmd.pitch = dumb.pitch;
    fbcmd.handle = dumb.handle;
    drm_ioctl(fd, DRM_IOCTL_MODE_ADDFB, &fbcmd);

    mdumb.handle = dumb.handle;
    drm_ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mdumb);

    fb->addr = mmap(0, dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mdumb.offset);
    fb->size = dumb.size;
    fb->fb_id = fbcmd.fb_id;
    fb->height = height;
    fb->width = width;

}

void drm_set_crtc(struct display * display){

	struct drm_mode_crtc crtc;
    memset(&crtc, 0, sizeof(struct drm_mode_crtc));
    crtc.mode = display->mode;
    crtc.crtc_id = display->crtc_id;
    crtc.count_connectors = 1;
    crtc.x = 0;
    crtc.y = 0;
    crtc.mode_valid = 1;
    crtc.set_connectors_ptr = display->ptr_connector_id;
    crtc.fb_id = display->fbs[0]->fb_id;
    drm_ioctl(display->fd, DRM_IOCTL_MODE_SETCRTC, &crtc);

}

void drm_page_flip(int fd, int fb_id, int crtc_id){

    struct drm_mode_crtc_page_flip flip;
    flip.fb_id = fb_id;
    flip.crtc_id = crtc_id;
    flip.flags = DRM_MODE_PAGE_FLIP_EVENT;
    flip.reserved = 0;
    ioctl(fd, DRM_IOCTL_MODE_PAGE_FLIP, &flip);

}

int open_dev(){
	int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    return fd;
}




void start_display(struct display * display){

    drm_set_crtc(display);
    drm_page_flip(display->fd, display->fbs[0]->fb_id, display->crtc_id);

}

void create_framebuffers(struct display * display){

    struct framebuffer ** fbs = (struct framebuffer **) malloc(sizeof(struct framebuffer *) * 2);
    for(int i=0;i<2;i++){
        fbs[i] = (struct framebuffer *)malloc(sizeof(struct framebuffer));
    }
    display->fbs = fbs;
    create_framebuffer(display->fd, (display->mode).hdisplay, (display->mode).vdisplay, fbs[0]);
    create_framebuffer(display->fd, (display->mode).hdisplay, (display->mode).vdisplay, fbs[1]);

}

void show_modes(int fd){

    struct drm_mode_card_res res;
	struct drm_mode_get_connector connector;

	drm_get_resources(fd, &res);

    int user_width, user_height;
    int no = 0;

	for(int i=0;i<res.count_connectors;i++){
		int connector_id = *((int *)((res.connector_id_ptr) + (i * 4)));
        drm_get_connector(fd, connector_id, &connector);
        if(connector.connection != connected){
            continue;
        }

		struct drm_mode_modeinfo * modes = (struct drm_mode_modeinfo *)connector.modes_ptr;
        for(int i=0;i<connector.count_modes;i++){
            printf("%d x %d\n", modes[i].hdisplay, modes[i].vdisplay);
        }
    }
}

struct display * choose_mode(int fd, int height, int width){

    struct drm_mode_card_res res;
    struct drm_mode_get_encoder enc;
	struct drm_mode_get_connector connector;
    struct drm_mode_modeinfo mode;
    struct display * display = NULL;

	drm_get_resources(fd, &res);

    int user_width = width, user_height = height;

	for(int i=0;i<res.count_connectors;i++){
		int connector_id = *((int *)((res.connector_id_ptr) + (i * 4)));
        drm_get_connector(fd, connector_id, &connector);
        if(connector.connection != connected){
            continue;
        }

		struct drm_mode_modeinfo * modes = (struct drm_mode_modeinfo *)connector.modes_ptr;
        for(int i=0;i<connector.count_modes;i++){
            if((modes[i].vdisplay == user_height && modes[i].hdisplay == user_width)){
                drm_get_encoder(fd, connector.encoder_id, &enc);
                display = (struct display *)malloc(sizeof(struct display));
                mode = modes[i];
                display->connector_id = connector_id;
                display->encoder_id = connector.encoder_id;
                display->crtc_id = enc.crtc_id;
                display->mode = mode;
                display->fd = fd;
                display->height = height;
                display->width = width;
                display->ptr_connector_id = (uint64_t)&(display->connector_id);
                return display;
            }
        }
    }
    return display;

}
