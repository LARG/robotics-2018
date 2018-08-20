#ifndef SPEECHBLOCK_4A6NIGLO
#define SPEECHBLOCK_4A6NIGLO

#include <memory/MemoryBlock.h>
#include <string>
#include <cstring>

#define SPEECH_TEXT_SIZE 80

struct SpeechBlock : public MemoryBlock {
  NO_SCHEMA(SpeechBlock);
 SpeechBlock():
  last_speech_frame_(0),
  say_text_(false)
    {
      header.version = 2;
      header.size = sizeof(SpeechBlock);
      
      text_[0] = '\0';
    }
  
  void say(std::string text) {
    strncpy(text_,text.c_str(),SPEECH_TEXT_SIZE-1);
    text_[SPEECH_TEXT_SIZE-1] = '\0';
    say_text_ = true;
  }

  unsigned last_speech_frame_;
  bool say_text_;
  char text_[SPEECH_TEXT_SIZE];
};

#endif /* end of include guard: SPEECHBLOCK_4A6NIGLO */
