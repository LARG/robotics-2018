#include <memory/StreamBuffer.h>
#include <inttypes.h>

#ifdef TOOL
#define BUFFER_SIZE_FORMAT PRIu64
#else
#define BUFFER_SIZE_FORMAT "llu"
#endif

#define VERIFY_RESIZE \
  if(n == 0 || n > 1'000'000'000'000) {\
    fprintf(stderr, "INVALID BUFFER REQUEST (%" BUFFER_SIZE_FORMAT ")!! The log file is most likely broken\n", n);\
    return;\
  }\

StreamBuffer::StreamBuffer() : size(0), capacity(0), buffer(NULL) {
}

StreamBuffer::StreamBuffer(unsigned char* b, IntT n) : size(n), capacity(n), buffer(b) {
}

StreamBuffer::StreamBuffer(char* b, IntT n) : size(n), capacity(n), buffer((unsigned char*)b) {
}

void StreamBuffer::clear() {
  if(buffer != NULL) delete [] buffer;
  capacity = 0;
  size = 0;
  buffer = NULL;
}

void StreamBuffer::rclear() {
  clear();
  for(auto child : children)
    child.rclear();
  children.clear();
}

void StreamBuffer::resize(IntT n) {
  if(capacity < n) {
    if(buffer != NULL)
      delete [] buffer;
    buffer = new unsigned char[n];
    capacity = n;
  }
}

void StreamBuffer::reset() {
  size = 0;
}

void StreamBuffer::add(const char* data, IntT n) {
  StreamBuffer sb;
  sb.read(data, n);
  children.push_back(sb);
}

void StreamBuffer::add(const unsigned char* data, IntT n) {
  StreamBuffer sb;
  sb.read(data, n);
  children.push_back(sb);
}

void StreamBuffer::add(std::istream& is, IntT n) {
  StreamBuffer sb;
  sb.read(is, n);
  children.push_back(sb);
}

void StreamBuffer::add(std::istream& is) {
  StreamBuffer sb;
  sb.read(is);
  children.push_back(sb);
}

void StreamBuffer::read(const unsigned char* data, IntT n) {
  VERIFY_RESIZE;
  resize(n);
  memcpy(buffer, data, n);
  size = n;
}

void StreamBuffer::read(const char* data, IntT n) {
  VERIFY_RESIZE;
  resize(n);
  memcpy((char*)buffer, data, n);
  size = n;
}

void StreamBuffer::read(std::istream& is, IntT n) {
  VERIFY_RESIZE;
  resize(n);
  is.read((char*)buffer, n);
  size = n;
}

void StreamBuffer::read(std::istream& is) {
  IntT n;
  is.read((char*)&n, sizeof(IntT));
  read(is, n);
}
  
void StreamBuffer::write(std::ostream& os) {
  write(os, this->size);
}

void StreamBuffer::write(std::ostream& os, IntT n) {
  os.write((char*)&n, sizeof(IntT));
  os.write((char*)this->buffer, n);
}

StreamBuffer StreamBuffer::combine(const std::vector<StreamBuffer>& buffers) {
  StreamBuffer sb;
  combine(buffers, sb);
  return sb;
}

void StreamBuffer::combine(const std::vector<StreamBuffer>& buffers, StreamBuffer& combined) {
  IntT totalSize = 0;
  for(auto buffer : buffers)
    totalSize += buffer.size;

  combined.resize(totalSize + buffers.size() * sizeof(IntT) + sizeof(IntT));
  IntT offset = 0;
  assert(buffers.size() < 0xFFFFFF);
  IntT size = buffers.size();
  memcpy((char*)combined.buffer, &size, sizeof(IntT));
  offset += sizeof(IntT);
  for(auto& buffer : buffers) {
    IntT *pieceSize = (IntT*)(combined.buffer + offset);
    *pieceSize = buffer.size;
    offset += sizeof(IntT);
    memcpy(combined.buffer + offset, buffer.buffer, buffer.size);
    offset += buffer.size;
  }
  combined.size = offset;
}

void StreamBuffer::combine(const std::list<StreamBuffer>& buffers, StreamBuffer& combined) {
  IntT totalSize = 0;
  for(auto buffer : buffers)
    totalSize += buffer.size;

  combined.resize(totalSize + buffers.size() * sizeof(IntT) + sizeof(IntT));
  IntT offset = 0;
  assert(buffers.size() < 0xFFFFFFFF);
  IntT size = buffers.size();
  memcpy((char*)combined.buffer, &size, sizeof(IntT));
  offset += sizeof(IntT);
  for(auto& buffer : buffers) {
    IntT *pieceSize = (IntT*)(combined.buffer + offset);
    *pieceSize = buffer.size;
    offset += sizeof(IntT);
    memcpy(combined.buffer + offset, buffer.buffer, buffer.size);
    offset += buffer.size;
  }
  combined.size = offset;
}

std::vector<StreamBuffer> StreamBuffer::separate() const {
  std::vector<StreamBuffer> separated;
  separate(*this, separated);
  return separated;
}

void StreamBuffer::contract() {
  combine(children, *this);
  clear(children);
  children.clear();
}

void StreamBuffer::expand() {
  separate(*this, children);
  clear();
}

void StreamBuffer::separate(const StreamBuffer& buffer, std::vector<StreamBuffer>& separated) {
  IntT offset = 0;
  IntT bufCount;
  memcpy((char*)&bufCount, buffer.buffer, sizeof(IntT));
  offset += sizeof(IntT);
  while(separated.size() < bufCount) {
    StreamBuffer piece;
    IntT *pieceSize = (IntT*)(buffer.buffer + offset);
    offset += sizeof(IntT);
    piece.read(buffer.buffer + offset, *pieceSize);
    offset += piece.size;
    separated.push_back(piece);
  }
}

void StreamBuffer::separate(const StreamBuffer& buffer, std::list<StreamBuffer>& separated) {
  IntT offset = 0;
  IntT bufCount;
  memcpy((char*)&bufCount, buffer.buffer, sizeof(IntT));
  offset += sizeof(IntT);
  while(separated.size() < bufCount) {
    StreamBuffer piece;
    IntT *pieceSize = (IntT*)(buffer.buffer + offset);
    offset += sizeof(IntT);
    piece.read(buffer.buffer + offset, *pieceSize);
    offset += piece.size;
    separated.push_back(piece);
  }
}

void StreamBuffer::clear(std::vector<StreamBuffer>& buffers) {
  for(auto sb : buffers) sb.clear();
}

void StreamBuffer::clear(std::list<StreamBuffer>& buffers) {
  for(auto sb : buffers) sb.clear();
}

