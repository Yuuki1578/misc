#define MISC_USE_GLOBAL_ALLOCATOR
#include "../Misc.h"

typedef struct {
  long double AxisX, AxisY;
} Square;

Square squareCreate(long double AxisX, long double AxisY) {
  return (Square){AxisX, AxisY};
}

int main(void) {
  ARENA_INIT();

  LinkedList SquareLink = {
      .ItemSize = sizeof(Square),
  };

  for (long double CrazyFloat = 0.000, Alter = 1.000; CrazyFloat < 12.515;
       CrazyFloat += 0.025, Alter -= 0.025) {
    Square NewSquare = squareCreate(CrazyFloat, Alter);
    linkedListPrepend(&SquareLink, &NewSquare);
  }

  FOR_LINK(SquareLink, Head) {
    Square *FromSquare = Head->Item;
    printf("Square [%Lf,%Lf] with diameter %Lf cm\n", FromSquare->AxisX,
           FromSquare->AxisY, FromSquare->AxisX * FromSquare->AxisY);
  }

  ARENA_DROP();
}
