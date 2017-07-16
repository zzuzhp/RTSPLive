# RTSPLive
media streaming server using RTSP

RTSPLive servers as a media streamer using RTSP

with it you could stream a media file(using RTSPRenderer.dll in GraphEdit, no additional codes are required),
or you could use RTSPLive.dll and feed it with frames(from media files or cameras etc.) to stream them.

Codecs
------

currently the server supports the following codec(s):

- [x] h264(avc)

Projects
--------

the solution incorporates 5 projects:

- [x] CLI (exe)

        -- a very simple test application, which feeds one h264 frame(I) repeatedly to the server. 
- [x] RTSPLive (dll)

        -- the RTSP streamer, which implements the RTSP protocol and accepts requests from different players.
- [x] RTSPRenderer (dll)   

        -- a directshow renderer filter(a renderer wrapper of RTSPLive.dll), which can be registered and used as the following screenshot shows.
- [x] UTE (dll)   

        -- UTE stands for Unified Transport Entry, which is a wrapper of ASIO(http://think-async.com/), this is our network library and it provides TCP/UDP transmissions.
- [x] XUtil (lib)

        -- some utils such as locks and threads ...

ScreenShots
-----------
use of RTSPRenderer.dll:
<img src="https://user-images.githubusercontent.com/8287989/27899746-1b4de86e-625e-11e7-825d-746927ccc978.PNG" height="304px" >
you can access the stream from either a vlc player:
<img src="https://user-images.githubusercontent.com/8287989/27899752-1e219284-625e-11e7-8fcb-1c2c4a6404b9.PNG" height="514px" >

or ffplay:
<img src="https://user-images.githubusercontent.com/8287989/27899756-20c5ca46-625e-11e7-93ba-1c8cabd5db68.PNG" height="443px" >

or any player on your mobile devices that support RTSP protocol:

<img src="https://user-images.githubusercontent.com/8287989/27899860-8d303824-625e-11e7-899e-8e26c12d6dac.jpeg" height="623px" >

and, this is the CLI project stream(played by vlc):

<img src="https://user-images.githubusercontent.com/8287989/27899957-ef571fea-625e-11e7-895c-bb4a10edaf94.PNG" height="414px" >

Compile
------
RTSPLive is developed on VS2015 (win7 64bit)

I only configured the debug(x86) solution, nothing else is tested...

RTSPRenderer.dll is a dshow render filter, to build it you need to:

1.   build dshow baseclasses(which is incorporated in the windows SDK).

2.   specify the include and library path to dshow for project 'RTSPRenderer'.

3.   register the filter by running command 'regsvr32' on the windows command shell(you'd better write a 'bat' file and run the file in 'Administrator mode').

4.   open GraphStudio.exe (http://blog.monogram.sk/janos/tools/monogram-graphstudio/) and insert a media file source filter that contains a H264 stream, and a demuxer, and our renderer named "RTSP Renderer".

5.   run the graph, and now the server is streaming the file.

RTSPRenderer is the only project that depends on the OS(windows), because it is a dshow filter :), other projects
should be OS independent.

UTE is the only project that has dependency on other open-source project(ASIO), which has already been included
in the solution(no need to download ASIO).

if you have any questions about RTSPLive, please let me known at: pengzhao218@126.com

Todo
------
client disconnection detection & RTCP & more RTSP commands & UDP optimization for multiplayers & more codecs ...
