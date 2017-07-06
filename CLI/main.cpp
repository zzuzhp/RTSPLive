#include "RTSPLive/RTSPCore.h"
#include "XUtil/XUTime.h"

int main()
{
    IRTSPLive * rtsp = RTSPLiveCreate(8554);
    rtsp->start();

    RTSP_avc_init_param init;
    init.pack       = RTSP_AVC_RFC;
    init.sps        = "\x67\x64\x00\x28\xac\x72\x04\x41\x60\x96\x84\x00\x00\x03\x00\x04\x00\x00\x03\x00\xf0\x3c\x60\xc6\x11\x80";
    init.sps_len    = 26;
    init.pps        = "\x68\xe8\x43\xb2\xc8\xb0";
    init.pps_len    = 6;

    uint32_t streamId = rtsp->add_stream(RTSP_MEDIA_AVC, &init);

    FILE * file = fopen("test.264", "rb");
    fseek(file, 0, SEEK_END);
    int data_len = ftell(file);
    fseek(file, 0, SEEK_SET);

    char * data = new char[data_len];
    fread(data, 1, data_len, file);

    RTSP_media_frame frame;
    
    frame.frame = data;
    frame.ts_ms = 0;
    frame.len   = data_len;
    frame.type  = RTSP_FRAME_UNCERTAIN;

    do
    {
        rtsp->push_stream(streamId, &frame);
        frame.ts_ms += 33;

        XUSleep(33);
    } while (1);

    RTSPLiveDestroy(rtsp);
	return 0;
}
