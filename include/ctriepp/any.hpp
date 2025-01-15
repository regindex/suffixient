#include <assert.h>
#include <cmath>
#include <map>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#define FATAL(message) std::cerr << "[FATAL] " << message << std::endl
#define ERROR(message) std::cerr << "[ERROR] " << message << std::endl
#define WARN(message) std::cerr << "[WARN] " << message << std::endl
#define INFO(message) std::cerr << "[INFO] " << message << std::endl
#ifdef NDEBUG
#define LOG(message) 0
#else
#define LOG(message) std::cerr << "[DEBUG] " << message << std::endl
#endif
#define TRACE(message)                                     \
    std::cerr << "[TRACE] " << __FILE__ << "/" << __LINE__ \
              << "/ " #message " : " << message << std::endl
#define COUT(message) std::cout << message << std::endl

typedef char Char;
typedef unsigned char Uchar;
typedef int32_t Int;
typedef uint32_t Uint;
typedef int64_t Long;
typedef u_int64_t Ulong;

#define INT_SIZE 32
#define CHAR_SIZE 8
#define LONG_SIZE 64
#define LONG_PAR_CHAR 8

#define DIV8(x) x >> 3
#define DIV256(x) x >> 8

constexpr static Uint EMPTY = -1;

// #define LONG_ALL_ONE static_cast<Ulong>(0xFFFFFFFFFFFFFFFFULL)
static const Ulong LONG_ALL_ONE = 0xFFFFFFFFFFFFFFFFULL;
static const Ulong PREFIX_MASK[] = {
    0x0000000000000000ULL, 0x00000000000000FFULL, 0x000000000000FFFFULL,
    0x0000000000FFFFFFULL, 0x00000000FFFFFFFFULL, 0x000000FFFFFFFFFFULL,
    0x0000FFFFFFFFFFFFULL, 0x00FFFFFFFFFFFFFFULL, LONG_ALL_ONE};
