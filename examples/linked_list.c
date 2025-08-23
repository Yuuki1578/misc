#include "../misc.h"

typedef struct {
    long double axis_x, axis_y;
} Square;

Square square_create(long double axis_x, long double axis_y)
{
    return (Square) { axis_x, axis_y };
}

void on_free(void* each_square, size_t square_size)
{
    Square* square = each_square;
    printf("Freeing square with x: %Lf, y: %Lf\n", square->axis_x, square->axis_y);
    (void)square_size;
}

int main(void)
{
    Linked_List square_link = {
        .item_size = sizeof(Square),
        .on_free = on_free,
    };

    for (long double crazy_float = 0.000, alter = 1.000; crazy_float < 1.000; crazy_float += 0.025, alter -= 0.025) {
        Square square = square_create(crazy_float, alter);
        ll_prepend(&square_link, &square);
    }

    ll_free(&square_link);
}
