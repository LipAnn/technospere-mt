#include <stdexcept>
#include <string>
#include <list>
#include <map>

using std::list;
using std::string;
using std::runtime_error;
using std::map;

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

struct Block {
    char *begin;
    size_t size;
    Block(char *b=nullptr, size_t s=0);
    bool empty() const;
    char* getEnd() const;
};

class Allocator;

class Pointer {
private:
    Allocator *allocator_;
    size_t id_;
public:
    Pointer(Allocator *alc=nullptr, size_t id=0);
    void* get() const; 
    size_t getSize() const;
    size_t getId() const;
};

class Allocator {
private:
    list<Block> free_blocks_;
    map<size_t, Block> pointers_;
    char *base1_;
    char *base2_;
    size_t memory_size1_;
    size_t memory_size2_;
    size_t pointer_id_;

    using BlockIterator = list<Block>::iterator;
    using PointerIterator = map<size_t, Block>::iterator;

    void splitBlock(BlockIterator, size_t);
    void extendBlock(BlockIterator, size_t);
    BlockIterator findNearestFreeBlock(Pointer&);
    BlockIterator addFreeBlock(Pointer&);
    BlockIterator merge2Blocks(BlockIterator, BlockIterator);
    void mergeBlocks(BlockIterator);
    void erasePointer(Pointer&);

public:
    Allocator(void*, size_t);
    
    Block findBlock(size_t);
    void* findBegin(size_t);
    size_t findSize(size_t);

    Pointer alloc(size_t);
    void realloc(Pointer&, size_t);
    void free(Pointer&);
    void defrag();

    string dump();
};

