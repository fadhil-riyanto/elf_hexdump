#include <stdint.h>
#include <stdlib.h>

struct data {
        int                     ptr;
};

void alloc_me(struct data *datax_ptr) {
        uint64_t *buf = (uint64_t *)malloc(sizeof(uint64_t) * 16);

}

int main() {
        struct data datax;
        alloc_me(&datax)
}