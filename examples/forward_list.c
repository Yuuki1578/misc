#define MISC_USE_GLOBAL_ALLOCATOR
#include "../misc.h"


typedef struct {
    long double axis_x, axis_y;
} Square;

Square square_create(long double axis_x, long double axis_y)
{
    return (Square){axis_x, axis_y};
}

int main(void)
{
    ARENA_INIT();

    ForwardList square_list = {
        .item_size = sizeof(Square),
    };

    for (long double crazy_float = 0.000, alterer = 1.000; crazy_float < 12.515; crazy_float += 0.025, alterer -= 0.025) {
        Square new_square = square_create(crazy_float, alterer);
        forward_list_prepend(&square_list, &new_square);
    }

    for (RawForwardList *each = square_list.head; each != NULL; each = each->next) {
        Square *item = each->item;
        printf("{ axis_x: %Lf, axis_y: %Lf }\n", item->axis_x, item->axis_y);
    }

    ARENA_DEINIT();
}
