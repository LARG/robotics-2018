#include <iostream>

#include <memory/SharedMemory.h>
#include <memory/FrameInfoBlock.h>

int notLive() {
 std::cout << "MEMORY NOT LIVE" << std::endl;
 return 1;
}

int main() {
 try {
   SharedMemory mem("TEAM0PLAYER1",false);
   FrameInfoBlock *block = (FrameInfoBlock*)mem.getBlockPtr("frame_info");
   if (block == NULL) {
     std::cout << "No memory block by that name" << std::endl;
     return notLive();
   }
   unsigned int id = block->frame_id;
   usleep(0.05 * 1000000);
   if (block->frame_id == id) {
     std::cout << id << " " << block->frame_id << std::endl;
     return notLive();
   }
   std::cout << "LIVE" << std::endl;
   return 0;
 } catch (...) {
   std::cout << "NO MEM FOUND" << std::endl;
   return notLive();
 }
}
