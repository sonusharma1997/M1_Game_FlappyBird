#include "unity.h"
#include <flappybird.h>

#define PROJECT_NAME    "Flappy Bird"

void test_collision(void);
void test_birdMove(void);
void test_score(void);

void setUp(){}

void tearDown(){}

int main()
{
  UNITY_BEGIN();


  RUN_TEST(test_collision);
  RUN_TEST(test_birdMove);
  RUN_TEST(test_score);

  return UNITY_END();
}

void test_collision(void) {
  int gaploc[]={200,100,100,50}; //location of gap [x,y,w,h] 
  int birdloc[]={210,50,20,20};//location of bird [x,y,w,h] 
  TEST_ASSERT_EQUAL(1, isColliding(gaploc, birdloc));
  
  /* Dummy fail*/
  int gaploc1[]={200,100,100,50}; //location of gap [x,y,w,h] 
  int birdloc1[]={100,50,20,20};//location of bird [x,y,w,h] 
  TEST_ASSERT_EQUAL(0, isColliding(gaploc1, birdloc1));
}

void test_birdMove(void) {
  
  int birdloc[]={210,50,20,20};//location of bird [x,y,w,h]
  int newheight=birdMove(gaploc, birdloc)[1];
  TEST_ASSERT_EQUAL(birdloc[1]-5, newheight );
 
}
