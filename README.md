This repo contains three sample code for exploring how multi-threading and inter-process/thread work under UNIX environment.

## How to use this code
You can use the included cmake file for compliation.
For part3 follows these steps:
1. under `~\Kernel` folder, run `make`
2. `sudo insmod pubsub.ko` (to remove the kernel module `sudo rmmod pubsub.ko')
3. under '~\Userspace` , run `make` and then run node.

You can start another terminal to run another instance of the object file to test adding and removing of the node from the publisher/subscriber lsit. This code supports a dynamic number of nodes in the runtime.
