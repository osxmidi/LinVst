For possible DAW plugin window focus options remove most of the // characters from the start of the lines in linvst.cpp around line 344

```

      case EnterNotify:
//      if(reaperid)
 //     if(mapped2)
  //    {     
      if(e.xcrossing.focus == False)
      {
      XSetInputFocus(display, child, RevertToPointerRoot, CurrentTime);
//    XSetInputFocus(display, child, RevertToParent, e.xcrossing.time);
      }
 //     }
      break;
      
      would end up as 
      
      case EnterNotify:
      if(reaperid)
      if(mapped2)
      {     
      if(e.xcrossing.focus == False)
      {
      XSetInputFocus(display, child, RevertToPointerRoot, CurrentTime);
//    XSetInputFocus(display, child, RevertToParent, e.xcrossing.time);
      }
      }
      break;
      
```
