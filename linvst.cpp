/*  linvst is based on wacvst Copyright 2009 retroware. All rights reserved. and dssi-vst Copyright 2004-2007 Chris Cannam

    linvst Mark White 2017

    This file is part of linvst.

    linvst is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "remotevstclient.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

extern "C" {

#define VST_EXPORT   __attribute__ ((visibility ("default")))

    extern VST_EXPORT AEffect * VSTPluginMain(audioMasterCallback audioMaster);

    AEffect * main_plugin (audioMasterCallback audioMaster) asm ("main");

#define main main_plugin

    VST_EXPORT AEffect * main(audioMasterCallback audioMaster)
    {
        return VSTPluginMain(audioMaster);
    }
}

#ifdef EMBED
#ifdef XEMBED

#define XEMBED_EMBEDDED_NOTIFY	0
#define XEMBED_FOCUS_OUT 5

void sendXembedMessage(Display* display, Window window, long message, long detail,
		long data1, long data2)
{
	XEvent event;

	memset(&event, 0, sizeof(event));
	event.xclient.type = ClientMessage;
	event.xclient.window = window;
	event.xclient.message_type = XInternAtom(display, "_XEMBED", false);
	event.xclient.format = 32;
	event.xclient.data.l[0] = CurrentTime;
	event.xclient.data.l[1] = message;
	event.xclient.data.l[2] = detail;
	event.xclient.data.l[3] = data1;
	event.xclient.data.l[4] = data2;

	XSendEvent(display, window, false, NoEventMask, &event);
//	XSync(display, false);
}

#ifdef EMBEDDRAG
void eventloopx(Display *display, Window parent, Window child, Window pparent, int eventrun2, int width, int height, int parentok, int reaperid, RemotePluginClient  *plugin)
#else
void eventloopx(Display *display, Window parent, Window child, int eventrun2, int width, int height, int reaperid, RemotePluginClient  *plugin)
#endif
{
#ifdef EMBEDDRAG
static  XEvent xevent;
static  XClientMessageEvent cm;
static  int accept = 0;
static int x2 = 0;
static int y2 = 0;
static	Atom XdndPosition = XInternAtom(display, "XdndPosition", False);
static	Atom XdndStatus = XInternAtom(display, "XdndStatus", False);
static	Atom XdndActionCopy = XInternAtom(display, "XdndActionCopy", False);
static	Atom XdndEnter = XInternAtom(display, "XdndEnter", False);
static	Atom XdndDrop = XInternAtom(display, "XdndDrop", False);
static	Atom XdndLeave = XInternAtom(display, "XdndLeave", False);
static  Atom XdndFinished = XInternAtom(display, "XdndFinished", False);
#endif
static int x = 0;
static int y = 0;
static Window ignored = 0;
static int mapped2 = 0;
#ifdef FOCUS
static int x3 = 0;
static int y3 = 0;
static Window ignored3 = 0;
#endif
static Atom xembedatom = XInternAtom(plugin->display, "_XEMBED_INFO", False);

     if((eventrun2 == 1) && parent && child && display)
      {
      int pending = XPending(display);

      for (int i=0; i<pending; i++)
      {
      XEvent e;

      XNextEvent(display, &e);

      switch (e.type)
      {
      case MapNotify:  
      if(e.xmap.window == child)
      mapped2 = 1;     
      break;	
		      
      case UnmapNotify:  
      if(e.xmap.window == child)
      mapped2 = 0;     
      break;	    
		      
      case EnterNotify:
//      if(reaperid)
      if(mapped2)
      {     
      if(e.xcrossing.focus == False)
      {
      XSetInputFocus(display, child, RevertToPointerRoot, CurrentTime);
//    XSetInputFocus(display, child, RevertToParent, e.xcrossing.time);
      }
      }
      break;
     
      case PropertyNotify:
      if (e.xproperty.atom == xembedatom) 
      {
#ifdef EMBEDTHREAD
      plugin->mapok = 1;
#else            
      XMapWindow(display, child);
      plugin->openGUI();
      XSync(display, false);
      XFlush(display);
#endif      
      }
      break;

#ifdef EMBEDDRAG
      case ClientMessage:
      if((e.xclient.message_type == XdndEnter) || (e.xclient.message_type == XdndPosition) || (e.xclient.message_type == XdndLeave) || (e.xclient.message_type == XdndDrop))
      {

      if(e.xclient.message_type == XdndPosition)
      {      
      x = 0;
      y = 0;
      ignored = 0;
            
      e.xclient.window = child;
      XSendEvent (display, child, False, NoEventMask, &e);
      
      XTranslateCoordinates(display, child, XDefaultRootWindow(display), 0, 0, &x, &y, &ignored);
      
      x2 = e.xclient.data.l[2]  >> 16;
      y2 = e.xclient.data.l[2] &0xffff;
            
      memset (&xevent, 0, sizeof(xevent));
      xevent.xany.type = ClientMessage;
      xevent.xany.display = display;
      xevent.xclient.window = e.xclient.data.l[0];
      xevent.xclient.message_type = XdndStatus;
      xevent.xclient.format = 32;
      xevent.xclient.data.l[0] = parent;
      if(((x2 >= x) && (x2 <= x + width)) && ((y2 >= y) && (y2 <= y + height)))
      {
      accept = 1;
      xevent.xclient.data.l[1] |= 1;
      }
      else
      {
      accept = 0;
      xevent.xclient.data.l[1] &= ~1;
      }
      xevent.xclient.data.l[4] = XdndActionCopy;

      XSendEvent (display, e.xclient.data.l[0], False, NoEventMask, &xevent);
    
      if(parentok && reaperid)
      {
      xevent.xclient.data.l[0] = pparent; 
      XSendEvent (display, e.xclient.data.l[0], False, NoEventMask, &xevent);
      }
      }
      else if(e.xclient.message_type == XdndDrop)
      {
      e.xclient.window = child;
      XSendEvent (display, child, False, NoEventMask, &e);

      memset(&cm, 0, sizeof(cm));
      cm.type = ClientMessage;
      cm.display = display;
      cm.window = e.xclient.data.l[0];
      cm.message_type = XdndFinished;
      cm.format=32;
      cm.data.l[0] = parent;
      cm.data.l[1] = accept;
      if(accept)
      cm.data.l[2] = XdndActionCopy;
      else
      cm.data.l[2] = None;
      XSendEvent(display, e.xclient.data.l[0], False, NoEventMask, (XEvent*)&cm);
      if(parentok && reaperid)
      {
      cm.data.l[0] = pparent;
      XSendEvent(display, e.xclient.data.l[0], False, NoEventMask, (XEvent*)&cm);
      }
      }
      else
      {
      e.xclient.window = child;
      XSendEvent (display, child, False, NoEventMask, &e);
      }

      }
      break;
#endif        
      default:
      break;                
        }
       }
      }
     }
#endif
#endif

#ifdef EMBED
#ifndef XEMBED
#ifdef EMBEDDRAG
void eventloop(Display *display, Window pparent, Window parent, Window child, int width, int height, int eventrun2, int parentok, int reaperid)
#else
void eventloop(Display *display, Window parent, Window child, int width, int height, int eventrun2, int reaperid)
#endif
{
#ifdef EMBEDDRAG
static  XEvent xevent;
static  XClientMessageEvent cm;
static  int accept = 0;
static int x2 = 0;
static int y2 = 0;
static	Atom XdndPosition = XInternAtom(display, "XdndPosition", False);
static	Atom XdndStatus = XInternAtom(display, "XdndStatus", False);
static	Atom XdndActionCopy = XInternAtom(display, "XdndActionCopy", False);
static	Atom XdndEnter = XInternAtom(display, "XdndEnter", False);
static	Atom XdndDrop = XInternAtom(display, "XdndDrop", False);
static	Atom XdndLeave = XInternAtom(display, "XdndLeave", False);
static  Atom XdndFinished = XInternAtom(display, "XdndFinished", False);
#endif
static int x = 0;
static int y = 0;
static Window ignored = 0;
static int mapped2 = 0;
#ifdef FOCUS
static int x3 = 0;
static int y3 = 0;
static Window ignored3 = 0;
#endif

     if(eventrun2 == 1)
      {
      if(parent && child && display)
      {

      int pending = XPending(display);

      for (int i=0; i<pending; i++)
      {
      XEvent e;

      XNextEvent(display, &e);

      switch (e.type)
      {
      case MapNotify:  
      if(e.xmap.window == child)
      mapped2 = 1;     
      break;	
		      
      case UnmapNotify:  
      if(e.xmap.window == child)
      mapped2 = 0;     
      break;	    
		      
      case EnterNotify:
//      if(reaperid)
      if(mapped2)
      {     
      if(e.xcrossing.focus == False)
      {
      XSetInputFocus(display, child, RevertToPointerRoot, CurrentTime);
//    XSetInputFocus(display, child, RevertToParent, e.xcrossing.time);
      }
      }
      break;
		      
#ifdef FOCUS
      case LeaveNotify:
      x3 = 0;
      y3 = 0;
      ignored3 = 0;            
      XTranslateCoordinates(display, child, XDefaultRootWindow(display), 0, 0, &x3, &y3, &ignored3);
  
      if(x3 < 0)
      { 
      width += x3;
      x3 = 0;
      }
  
      if(y3 < 0)
      {
      height += y3;
      y3 = 0;;
      }
		      
      if(((e.xcrossing.x_root < x3) || (e.xcrossing.x_root > x3 + (width - 1))) || ((e.xcrossing.y_root < y3) || (e.xcrossing.y_root > y3 + (height - 1))))      
      {
      if(mapped2)
      {
      if(parentok && reaperid)
      XSetInputFocus(display, pparent, RevertToPointerRoot, CurrentTime);
      else
      XSetInputFocus(display, PointerRoot, RevertToPointerRoot, CurrentTime);   
      }
      }      
      break; 
#endif
		      	
      case ConfigureNotify:
//      if((e.xconfigure.event == parent) || (e.xconfigure.event == child) || ((e.xconfigure.event == pparent) && (parentok)))
//      {
      x = 0;
      y = 0;
      ignored = 0;

      XTranslateCoordinates(display, parent, XDefaultRootWindow(display), 0, 0, &x, &y, &ignored);
      e.xconfigure.send_event = false;
      e.xconfigure.type = ConfigureNotify;
      e.xconfigure.event = child;
      e.xconfigure.window = child;
      e.xconfigure.x = x;
      e.xconfigure.y = y;
      e.xconfigure.width = width;
      e.xconfigure.height = height;
      e.xconfigure.border_width = 0;
      e.xconfigure.above = None;
      e.xconfigure.override_redirect = False;
      XSendEvent (display, child, False, StructureNotifyMask | SubstructureRedirectMask, &e);
//      }
      break;

#ifdef EMBEDDRAG
      case ClientMessage:
      if((e.xclient.message_type == XdndEnter) || (e.xclient.message_type == XdndPosition) || (e.xclient.message_type == XdndLeave) || (e.xclient.message_type == XdndDrop))
      {

      if(e.xclient.message_type == XdndPosition)
      {      
      x = 0;
      y = 0;
      ignored = 0;
            
      e.xclient.window = child;
      XSendEvent (display, child, False, NoEventMask, &e);
      
      XTranslateCoordinates(display, child, XDefaultRootWindow(display), 0, 0, &x, &y, &ignored);
      
      x2 = e.xclient.data.l[2]  >> 16;
      y2 = e.xclient.data.l[2] &0xffff;
            
      memset (&xevent, 0, sizeof(xevent));
      xevent.xany.type = ClientMessage;
      xevent.xany.display = display;
      xevent.xclient.window = e.xclient.data.l[0];
      xevent.xclient.message_type = XdndStatus;
      xevent.xclient.format = 32;
      xevent.xclient.data.l[0] = parent;
      if(((x2 >= x) && (x2 <= x + width)) && ((y2 >= y) && (y2 <= y + height)))
      {
      accept = 1;
      xevent.xclient.data.l[1] |= 1;
      }
      else
      {
      accept = 0;
      xevent.xclient.data.l[1] &= ~1;
      }
      xevent.xclient.data.l[4] = XdndActionCopy;

      XSendEvent (display, e.xclient.data.l[0], False, NoEventMask, &xevent);
    
      if(parentok && reaperid)
      {
      xevent.xclient.data.l[0] = pparent; 
      XSendEvent (display, e.xclient.data.l[0], False, NoEventMask, &xevent);
      }
      }
      else if(e.xclient.message_type == XdndDrop)
      {
      e.xclient.window = child;
      XSendEvent (display, child, False, NoEventMask, &e);

      memset(&cm, 0, sizeof(cm));
      cm.type = ClientMessage;
      cm.display = display;
      cm.window = e.xclient.data.l[0];
      cm.message_type = XdndFinished;
      cm.format=32;
      cm.data.l[0] = parent;
      cm.data.l[1] = accept;
      if(accept)
      cm.data.l[2] = XdndActionCopy;
      else
      cm.data.l[2] = None;
      XSendEvent(display, e.xclient.data.l[0], False, NoEventMask, (XEvent*)&cm);
      if(parentok && reaperid)
      {
      cm.data.l[0] = pparent;
      XSendEvent(display, e.xclient.data.l[0], False, NoEventMask, (XEvent*)&cm);
      }
      }
      else
      {
      e.xclient.window = child;
      XSendEvent (display, child, False, NoEventMask, &e);
      }

      }
      break;
#endif

      default:
      break;
         }
        }
      }
     }
    }
#endif
#endif

VstIntPtr dispatcher(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
    RemotePluginClient  *plugin = (RemotePluginClient *) effect->object;
    VstIntPtr           v = 0;
    static ERect        *rp;

#ifdef EMBED
#ifdef EMBEDDRAG
    static Atom XdndProxy;
    static Atom XdndAware;
    static Atom version;
    static XSetWindowAttributes attr = {0};
#endif
#endif

#ifdef EMBED
#ifdef TRACKTIONWM  
static char dawbuf[512];
#endif
#endif
   
#ifdef XEMBED
    static Atom xembedatom;
    static   unsigned long data[2];
#endif
	
    if(!plugin)
    return 0;
	
    if(plugin->m_inexcept == 1)
    {
    return 0;
    }
	
    if(plugin->m_threadbreak == 1)
    {
    return 0;
    }    	

    switch (opcode)
    {
    case effEditGetRect:
    {
        rp = &plugin->retRect;
        *((struct ERect **)ptr) = rp;
    }
        break;

    case effEditIdle:
#ifdef EMBED
#ifdef XEMBED
#ifdef EMBEDDRAG
       if(plugin->eventrun == 1)
       eventloopx(plugin->display, plugin->parent, plugin->child, plugin->pparent, plugin->eventrun, plugin->width, plugin->height, plugin->parentok, plugin->reaperid, plugin);
#else
       if(plugin->eventrun == 1)
       eventloopx(plugin->display, plugin->parent, plugin->child, plugin->eventrun, plugin->width, plugin->height, plugin->reaperid, plugin);       
#endif
#else
#ifdef EMBEDDRAG
        if(plugin->eventrun == 1)
        eventloop(plugin->display, plugin->pparent, plugin->parent, plugin->child, plugin->width, plugin->height, plugin->eventrun, plugin->parentok, plugin->reaperid);
#else
        if(plugin->eventrun == 1)
        eventloop(plugin->display, plugin->parent, plugin->child, plugin->width, plugin->height, plugin->eventrun, plugin->reaperid);
#endif
#endif
#endif
        v = plugin->effVoidOp2(effEditIdle, index, value, opt);
        break;

    case effStartProcess:
        plugin->effVoidOp(effStartProcess);
        break;

    case effStopProcess:
        plugin->effVoidOp(effStopProcess);
        break;
		    
    case effMainsChanged:
        plugin->effVoidOp2(effMainsChanged, index, value, opt);
        break;

    case effGetVendorString:
        strncpy((char *) ptr, plugin->getMaker().c_str(), kVstMaxVendorStrLen);
        break;

    case effGetEffectName:
        strncpy((char *) ptr, plugin->getName().c_str(), kVstMaxEffectNameLen);
        break;

    case effGetParamName:
      //  strncpy((char *) ptr, plugin->getParameterName(index).c_str(), kVstMaxParamStrLen);
	strncpy((char *) ptr, plugin->getParameterName(index).c_str(), kVstMaxVendorStrLen);
        break;

    case effGetParamLabel:
    //    plugin->getEffString(effGetParamLabel, index, (char *) ptr, kVstMaxParamStrLen);
        plugin->getEffString(effGetParamLabel, index, (char *) ptr, kVstMaxVendorStrLen);
        break;

    case effGetParamDisplay:
     //   plugin->getEffString(effGetParamDisplay, index, (char *) ptr, kVstMaxParamStrLen);
        plugin->getEffString(effGetParamDisplay, index, (char *) ptr, kVstMaxVendorStrLen);
        break;

    case effGetProgramNameIndexed:
        strncpy((char *) ptr, plugin->getProgramNameIndexed(index).c_str(), kVstMaxProgNameLen);
        break;

    case effGetProgramName:
        strncpy((char *) ptr, plugin->getProgramName().c_str(), kVstMaxProgNameLen);
        break;

    case effSetSampleRate:
        plugin->setSampleRate(opt);
        break;

    case effSetBlockSize:
        plugin->setBufferSize ((VstInt32)value);
        break;

#ifdef DOUBLEP
    case effSetProcessPrecision:
        v = plugin->setPrecision(value);
      break;  
#endif

#ifdef MIDIEFF   
    case effGetInputProperties:    
        v = plugin->getEffInProp(index, (char *)ptr); 
        break;  
        
    case effGetOutputProperties:    
        v = plugin->getEffOutProp(index, (char *)ptr);
        break; 
        
    case effGetMidiKeyName:    
        v = plugin->getEffMidiKey(index, (char *) ptr);
        break;   
        
    case effGetMidiProgramName:    
        v = plugin->getEffMidiProgName(index, (char *) ptr);
        break;   
        
    case effGetCurrentMidiProgram:    
        v = plugin->getEffMidiCurProg(index, (char *) ptr);
        break; 
        
    case effGetMidiProgramCategory:    
        v = plugin->getEffMidiProgCat(index, (char *) ptr);
        break;   
        
    case effHasMidiProgramsChanged:    
        v = plugin->getEffMidiProgCh(index);
        break;   
                 
    case effSetSpeakerArrangement:
        v = plugin->setEffSpeaker(value, (char *)ptr);
      break;
      
    case effGetSpeakerArrangement:
        v = plugin->getEffSpeaker(value, (char *)ptr);
      break;  
#endif
		    
    case effGetVstVersion:
        v = kVstVersion;
        break;

    case effGetPlugCategory:
        v = plugin->getEffInt(effGetPlugCategory);
        break;
#ifdef WAVES
    case effShellGetNextPlugin:
        v = plugin->getShellName((char *) ptr);
        break;
#endif
    case effSetProgram:
        plugin->setCurrentProgram((VstInt32)value);
        break;

    case effEditOpen:
#ifdef EMBED
#ifdef XEMBED
    {
        plugin->showGUI();
       // usleep(50000);

        plugin->handle = plugin->winm.handle;
        plugin->width = plugin->winm.width;
        plugin->height = plugin->winm.height;
        plugin->parent = (Window) ptr;
        plugin->child = (Window) plugin->handle;

        rp = &plugin->retRect;
        rp->bottom = plugin->height;
        rp->top = 0;
        rp->right = plugin->width;
        rp->left = 0;

        plugin->display = XOpenDisplay(0);

        if(plugin->display && plugin->handle)
        {	    
     //   XResizeWindow(plugin->display, plugin->parent, plugin->width, plugin->height);
     
       xembedatom = XInternAtom(plugin->display, "_XEMBED_INFO", False);
                
       data[0] = 0;
       data[1] = 1;
                
       XChangeProperty(plugin->display, plugin->child, xembedatom, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) data, 2);  
       
#ifdef EMBEDDRAG
       plugin->root = 0;
       plugin->children = 0;
       plugin->numchildren = 0;

       attr = {0};
       attr.event_mask = NoEventMask;

       plugin->x11_win = XCreateWindow(plugin->display, DefaultRootWindow(plugin->display), 0, 0, 1, 1, 0, 0, InputOnly, CopyFromParent, CWEventMask, &attr);

       if(plugin->x11_win)
       {
       XdndProxy = XInternAtom(plugin->display, "XdndProxy", False);

       XdndAware = XInternAtom(plugin->display, "XdndAware", False);
       version = 5;
       XChangeProperty(plugin->display, plugin->x11_win, XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&version, 1);

       plugin->parentok = 0;

       if(XQueryTree(plugin->display, plugin->parent, &plugin->root, &plugin->pparent, &plugin->children, &plugin->numchildren) != 0)
       {
       if(plugin->children)
       XFree(plugin->children);
       if((plugin->pparent != plugin->root) && (plugin->pparent != 0))
       plugin->parentok = 1;
       }
	       
       if(plugin->parentok == 0)
       plugin->pparent = 0;
       
       if(plugin->parentok && plugin->reaperid)
       XChangeProperty(plugin->display, plugin->pparent, XdndProxy, XA_WINDOW, 32, PropModeReplace, (unsigned char*)&plugin->x11_win, 1);
       else
       XChangeProperty(plugin->display, plugin->parent, XdndProxy, XA_WINDOW, 32, PropModeReplace, (unsigned char*)&plugin->x11_win, 1);

       XChangeProperty(plugin->display, plugin->x11_win, XdndProxy, XA_WINDOW, 32, PropModeReplace, (unsigned char*)&plugin->x11_win, 1);
       }
       #endif
	       
#ifdef FOCUS
      XSelectInput(plugin->display, plugin->parent, SubstructureRedirectMask | StructureNotifyMask | SubstructureNotifyMask);
      XSelectInput(plugin->display, plugin->child, SubstructureRedirectMask | StructureNotifyMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask | PropertyChangeMask); 
#else 
      XSelectInput(plugin->display, plugin->parent, SubstructureRedirectMask | StructureNotifyMask | SubstructureNotifyMask);
      XSelectInput(plugin->display, plugin->child, SubstructureRedirectMask | StructureNotifyMask | SubstructureNotifyMask | EnterWindowMask | PropertyChangeMask);	   
#endif
	       
       plugin->eventrun = 1;   
       
      XReparentWindow(plugin->display, plugin->child, plugin->parent, 0, 0);
      
     sendXembedMessage(plugin->display, plugin->child, XEMBED_EMBEDDED_NOTIFY, 0, plugin->parent, 0);      
              
#ifdef EMBEDTHREAD
        plugin->runembed = 1;  
#else

  //    usleep(250000);
 
    //  usleep(250000);

   //   XMapWindow(plugin->display, plugin->child);
  //    XSync(plugin->display, false);
   //   XFlush(plugin->display);
         
   //  usleep(250000);
       
    //  plugin->openGUI();
#endif            
        plugin->displayerr = 0;
        }
       else
       {
       plugin->displayerr = 1;
       plugin->display = 0;
       }
     }   
#else
    {
        plugin->showGUI();
      //  usleep(50000);

       plugin->handle = plugin->winm.handle;
       plugin->width = plugin->winm.width;
       plugin->height = plugin->winm.height;
       plugin->parent = (Window) ptr;
       plugin->child = (Window) plugin->handle;

       rp = &plugin->retRect;
       rp->bottom = plugin->height;
       rp->top = 0;
       rp->right = plugin->width;
       rp->left = 0;

       plugin->display = XOpenDisplay(0);

       if(plugin->display && plugin->handle)
       {
#ifdef EMBEDDRAG
       plugin->root = 0;
       plugin->children = 0;
       plugin->numchildren = 0;

       attr = {0};
       attr.event_mask = NoEventMask;

       plugin->x11_win = XCreateWindow(plugin->display, DefaultRootWindow(plugin->display), 0, 0, 1, 1, 0, 0, InputOnly, CopyFromParent, CWEventMask, &attr);

       if(plugin->x11_win)
       {
       XdndProxy = XInternAtom(plugin->display, "XdndProxy", False);

       XdndAware = XInternAtom(plugin->display, "XdndAware", False);
       version = 5;
       XChangeProperty(plugin->display, plugin->x11_win, XdndAware, XA_ATOM, 32, PropModeReplace, (unsigned char*)&version, 1);

       plugin->parentok = 0;

       if(XQueryTree(plugin->display, plugin->parent, &plugin->root, &plugin->pparent, &plugin->children, &plugin->numchildren) != 0)
       {
       if(plugin->children)
       XFree(plugin->children);
       if((plugin->pparent != plugin->root) && (plugin->pparent != 0))
       plugin->parentok = 1;
       }
	       
       if(plugin->parentok == 0)
       plugin->pparent = 0;
       
       if(plugin->parentok && plugin->reaperid)
       XChangeProperty(plugin->display, plugin->pparent, XdndProxy, XA_WINDOW, 32, PropModeReplace, (unsigned char*)&plugin->x11_win, 1);
       else
       XChangeProperty(plugin->display, plugin->parent, XdndProxy, XA_WINDOW, 32, PropModeReplace, (unsigned char*)&plugin->x11_win, 1);

       XChangeProperty(plugin->display, plugin->x11_win, XdndProxy, XA_WINDOW, 32, PropModeReplace, (unsigned char*)&plugin->x11_win, 1);
       }
       #endif
	       
#ifdef FOCUS
      XSelectInput(plugin->display, plugin->parent, SubstructureRedirectMask | StructureNotifyMask | SubstructureNotifyMask);
      XSelectInput(plugin->display, plugin->child, SubstructureRedirectMask | StructureNotifyMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask); 
#else 
      XSelectInput(plugin->display, plugin->parent, SubstructureRedirectMask | StructureNotifyMask | SubstructureNotifyMask);
      XSelectInput(plugin->display, plugin->child, SubstructureRedirectMask | StructureNotifyMask | SubstructureNotifyMask | EnterWindowMask);	   
#endif
	       
       plugin->eventrun = 1;

       XSync(plugin->display, false);
       XFlush(plugin->display);

  //     XResizeWindow(plugin->display, plugin->parent, plugin->width, plugin->height);

#ifdef EMBEDTHREAD
        plugin->runembed = 1;  
#else
       usleep(100000);

       XReparentWindow(plugin->display, plugin->child, plugin->parent, 0, 0);

       XMapWindow(plugin->display, plugin->child);
       XSync(plugin->display, false);
       XFlush(plugin->display);

 //     usleep(100000);

       plugin->openGUI();
#endif          
       plugin->displayerr = 0;   
       }
       else
       {
       plugin->displayerr = 1;
       plugin->display = 0;
       plugin->eventrun = 0;
       }
     }
#endif
#else
        plugin->showGUI();
#endif
        break;

    case effEditClose:
#ifdef EMBED
/*
#ifdef EMBEDTHREAD
if(plugin->runembed == 1)
{
    while(plugin->runembed == 1)
    usleep(1000);
    usleep(50000);
}
#endif
*/	    
        plugin->eventrun = 0;   
        
        if(plugin->displayerr == 1)
        break;

        plugin->hideGUI();  

        if(plugin->display)
        {
#ifdef EMBEDDRAG
        if(plugin->x11_win)
        XDestroyWindow (plugin->display, plugin->x11_win);
        plugin->x11_win = 0;
#endif        
        XSync(plugin->display, false);
        XCloseDisplay(plugin->display);
        plugin->display = 0;
        }   
#else            
        plugin->hideGUI();
#endif            
        break;

    case effCanDo:
        if (ptr && !strcmp((char *)ptr,"hasCockosExtensions"))
	{
	plugin->reaperid = 1;
        plugin->effVoidOp(78345432);
	}
#ifdef EMBED
#ifdef TRACKTIONWM    
        if(plugin->reaperid == 0)
        {
        if(plugin->theEffect && plugin->m_audioMaster)
        {
        plugin->m_audioMaster(plugin->theEffect, audioMasterGetProductString, 0, 0, dawbuf, 0);
        if((strcmp(dawbuf, "Tracktion") == 0) || (strcmp(dawbuf, "Waveform") == 0))
        plugin->effVoidOp(67584930);
        }
        }
#endif
#endif
#ifdef CANDOEFF
        v = plugin->getEffCanDo((char *)ptr);     
#else		    
        v = 1;
#endif		    
        break;

    case effProcessEvents:
        v = plugin->processVstEvents((VstEvents *) ptr);
        break;

    case effGetChunk:
        v = plugin->getChunk((void **) ptr, index);
        break;

    case effSetChunk:
        v = plugin->setChunk(ptr, value, index);
        break;

    case effGetProgram:
        v = plugin->getProgram();
        break;
		    
    case effCanBeAutomated:
        v = plugin->canBeAutomated(index);
        break;

        case effOpen:
        plugin->EffectOpen();
        break;
	    
    case effClose:
#ifdef EMBED    
        plugin->eventrun = 0;
#endif
        plugin->effVoidOp(effClose);

#ifdef EMBED
        if(plugin->display)
        {
#ifdef EMBEDDRAG
        if(plugin->x11_win)
        XDestroyWindow (plugin->display, plugin->x11_win);
        plugin->x11_win = 0;
#endif        
        XSync(plugin->display, false);
        XCloseDisplay(plugin->display);
        plugin->display = 0;
        }
#endif

/*
#ifdef AMT
        if(plugin->m_shm3)
        {
            for(int i=0;i<500;i++)
            {
                usleep(10000);
                if(plugin->m_threadbreakexit)
                break;
            }
        }
        else
        {
            usleep(500000);
        }
#else
        usleep(500000);
#endif
*/
        delete plugin;		    
        break;

    default:
        break;
    }
    return v;
}

void processDouble(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames)
{
#ifdef DOUBLEP
    RemotePluginClient *plugin = (RemotePluginClient *) effect->object;

    if(!plugin)
    return;
    
    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        plugin->processdouble(inputs, outputs, sampleFrames);
#endif	
    return;
}

void process(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames)
{
    RemotePluginClient *plugin = (RemotePluginClient *) effect->object;

    if(!plugin)
    return;
    
    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        plugin->process(inputs, outputs, sampleFrames);
    return;
}

void setParameter(AEffect* effect, VstInt32 index, float parameter)
{
    RemotePluginClient *plugin = (RemotePluginClient *) effect->object;

    if(!plugin)
    return;
    
    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        plugin->setParameter(index, parameter);
    return;
}

float getParameter(AEffect* effect, VstInt32 index)
{
    RemotePluginClient  *plugin = (RemotePluginClient *) effect->object;
    float               retval = -1;

    if(!plugin)
    return retval;
    
    if((plugin->m_bufferSize > 0) && (plugin->m_numInputs >= 0) && (plugin->m_numOutputs >= 0))
        retval = plugin->getParameter(index);
    return retval;
}

void initEffect(AEffect *eff, RemotePluginClient *plugin)
{
    memset(eff, 0x0, sizeof(AEffect));
    eff->magic = kEffectMagic;
    eff->dispatcher = dispatcher;
    eff->setParameter = setParameter;
    eff->getParameter = getParameter;
    eff->numInputs = plugin->getInputCount();
    eff->numOutputs = plugin->getOutputCount();
    eff->numPrograms = plugin->getProgramCount();
    eff->numParams = plugin->getParameterCount();
    eff->flags = plugin->getFlags();
#ifndef DOUBLEP
    eff->flags &= ~effFlagsCanDoubleReplacing;
    eff->flags |= effFlagsCanReplacing;
#endif
    eff->resvd1 = 0;
    eff->resvd2 = 0;
    eff->initialDelay = plugin->getinitialDelay();
    eff->object = (void *) plugin;
    eff->user = 0;
    eff->uniqueID = plugin->getUID();
    eff->version = 100;
    eff->processReplacing = process;
    eff->processDoubleReplacing = processDouble;
}

void errwin2()
{
static Window window = 0;
static Window ignored = 0;
static Display* display = 0;
static int screen = 0;
static Atom winstate;
static Atom winmodal;
    
std::string filename2;

  filename2 = "lin-vst-server/vst not found or LinVst version mismatch";
      
  XInitThreads();
  display = XOpenDisplay(NULL);  
  if (!display) 
  return;  
  screen = DefaultScreen(display);
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 480, 20, 0, BlackPixel(display, screen), WhitePixel(display, screen));
  if (!window) 
  return;
  winstate = XInternAtom(display, "_NET_WM_STATE", True);
  winmodal = XInternAtom(display, "_NET_WM_STATE_ABOVE", True);
  XChangeProperty(display, window, winstate, XA_ATOM, 32, PropModeReplace, (unsigned char*)&winmodal, 1);
  XStoreName(display, window, filename2.c_str()); 
  XMapWindow(display, window);
  XSync (display, false);
  XFlush(display);
  sleep(10);
  XSync (display, false);
  XFlush(display);
  XDestroyWindow(display, window);
  XCloseDisplay(display);  
  }

VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster)
{
    RemotePluginClient *plugin;

    if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
        return 0;

    try
    {
        plugin = new RemoteVSTClient(audioMaster);
    }
    catch (std::string e)
    {
        std::cerr << "Could not connect to Server" << std::endl;
	errwin2();    
        if(plugin)
        {
        plugin->m_runok = 1;
        delete plugin;
        }
        return 0;
    }
		
    if(plugin->m_runok == 1)
    {
        std::cerr << "LinVst Error: lin-vst-server not found or vst dll load timeout or LinVst version mismatch" << std::endl;
        errwin2();
	if(plugin)
        delete plugin;
        return 0;
    }

    initEffect(plugin->theEffect, plugin);
	
#ifdef EMBED
        XInitThreads();
#endif

    return plugin->theEffect;
}

