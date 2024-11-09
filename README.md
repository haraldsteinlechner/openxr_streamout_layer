# OpenXR Streamout Layer

This implementation provides an OpenXR Layer implementation which can be used grab and stream out openxr rendered output to third party applications, recording applications (e.g. obs), 3D screens, over the network etc.

As a proof of concept to explore the idea, we implemented a rather naive approach to streaming framebuffers over websocket and display them in a separate 3D stereo window.

Here is an overview on possible implementations: 
| Name          | Transport mechanism    | Latency/overhead    | Portability    | Effort | Encoding mechanism | Availability                         |
| --            | --                     | --                  | --             | -      | --                 | --                                   |  
| WebSocket     | Local net work & Web   | highest             | highest        | low    | Image compression  | local & nonlocal, multiview possible | 
| Memory Mapped | IPC                    | low                 | high           | medium | none               | local pc                             | 
| NDI           | IPC                    | low                 | high           | medium | NDI                | Local & network & potentially cloud  | 
| Ext Mem       | IPC & Resource Sharing | almost none         | bad            | high   | none               | local pc                             |

