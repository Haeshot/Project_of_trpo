#include <stdio.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include "game_logic.h"   // Ваш заголовок с объявлениями

// ------------------------------------------------------------
// Тесты для GetLocationByScore
// ------------------------------------------------------------
void test_forest_location(void) {
    // Лес: высота 0..999
    CU_ASSERT_EQUAL(GetLocationByScore(0), LOC_FOREST);
    CU_ASSERT_EQUAL(GetLocationByScore(500), LOC_FOREST);
    CU_ASSERT_EQUAL(GetLocationByScore(999), LOC_FOREST);
}

void test_desert_location(void) {
    // Пустыня: 1000..1999
    CU_ASSERT_EQUAL(GetLocationByScore(1000), LOC_DESERT);
    CU_ASSERT_EQUAL(GetLocationByScore(1500), LOC_DESERT);
    CU_ASSERT_EQUAL(GetLocationByScore(1999), LOC_DESERT);
}

void test_snow_location(void) {
    // Снег: 2000..2999
    CU_ASSERT_EQUAL(GetLocationByScore(2000), LOC_SNOW);
    CU_ASSERT_EQUAL(GetLocationByScore(2500), LOC_SNOW);
    CU_ASSERT_EQUAL(GetLocationByScore(2999), LOC_SNOW);
}

void test_space_location(void) {
    // Космос: 3000..3999
    CU_ASSERT_EQUAL(GetLocationByScore(3000), LOC_SPACE);
    CU_ASSERT_EQUAL(GetLocationByScore(3500), LOC_SPACE);
    CU_ASSERT_EQUAL(GetLocationByScore(3999), LOC_SPACE);
}

void test_location_cyclic(void) {
    // Цикличность: 4000 -> лес, 5000 -> пустыня и т.д.
    CU_ASSERT_EQUAL(GetLocationByScore(4000), LOC_FOREST);
    CU_ASSERT_EQUAL(GetLocationByScore(4500), LOC_FOREST);
    CU_ASSERT_EQUAL(GetLocationByScore(5000), LOC_DESERT);
    CU_ASSERT_EQUAL(GetLocationByScore(6000), LOC_SNOW);
    CU_ASSERT_EQUAL(GetLocationByScore(7000), LOC_SPACE);
    CU_ASSERT_EQUAL(GetLocationByScore(8000), LOC_FOREST);
}

// ------------------------------------------------------------
// Тесты для GetUITextColor
// ------------------------------------------------------------
void test_ui_forest_color(void) {
    Color c = GetUITextColor(LOC_FOREST);
    CU_ASSERT_EQUAL(c.r, DARKGRAY.r);
    CU_ASSERT_EQUAL(c.g, DARKGRAY.g);
    CU_ASSERT_EQUAL(c.b, DARKGRAY.b);
    CU_ASSERT_EQUAL(c.a, DARKGRAY.a);
}

void test_ui_desert_color(void) {
    Color expected = { 60, 20, 20, 255 };
    Color c = GetUITextColor(LOC_DESERT);
    CU_ASSERT_EQUAL(c.r, expected.r);
    CU_ASSERT_EQUAL(c.g, expected.g);
    CU_ASSERT_EQUAL(c.b, expected.b);
    CU_ASSERT_EQUAL(c.a, expected.a);
}

void test_ui_snow_color(void) {
    Color expected = { 20, 40, 80, 255 };
    Color c = GetUITextColor(LOC_SNOW);
    CU_ASSERT_EQUAL(c.r, expected.r);
    CU_ASSERT_EQUAL(c.g, expected.g);
    CU_ASSERT_EQUAL(c.b, expected.b);
    CU_ASSERT_EQUAL(c.a, expected.a);
}

void test_ui_space_color(void) {
    Color c = GetUITextColor(LOC_SPACE);
    CU_ASSERT_EQUAL(c.r, RAYWHITE.r);
    CU_ASSERT_EQUAL(c.g, RAYWHITE.g);
    CU_ASSERT_EQUAL(c.b, RAYWHITE.b);
    CU_ASSERT_EQUAL(c.a, RAYWHITE.a);
}

// ------------------------------------------------------------
// Запуск тестов
// ------------------------------------------------------------
int main() {
    CU_pSuite suite = NULL;
    
    // Инициализация реестра CUnit
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();
    
    // Добавление набора тестов
    suite = CU_add_suite("SkyJumper_Logic", NULL, NULL);
    if (!suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    // Добавление тестов для GetLocationByScore
    CU_add_test(suite, "test_forest_location", test_forest_location);
    CU_add_test(suite, "test_desert_location", test_desert_location);
    CU_add_test(suite, "test_snow_location", test_snow_location);
    CU_add_test(suite, "test_space_location", test_space_location);
    CU_add_test(suite, "test_location_cyclic", test_location_cyclic);
    
    // Добавление тестов для GetUITextColor
    CU_add_test(suite, "test_ui_forest_color", test_ui_forest_color);
    CU_add_test(suite, "test_ui_desert_color", test_ui_desert_color);
    CU_add_test(suite, "test_ui_snow_color", test_ui_snow_color);
    CU_add_test(suite, "test_ui_space_color", test_ui_space_color);
    
    // Запуск тестов
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    
    // Очистка
    CU_cleanup_registry();
    return CU_get_error();
}