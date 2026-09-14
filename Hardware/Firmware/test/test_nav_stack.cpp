#include <cassert>
#include <cstdio>
#include <cstring>
#include "../src/app/nav_state.h"
#include "../src/app/nav_stack.h"

void test_initial_state() {
    NavStack stack;
    stack.init();
    assert(stack.get_depth() == 0);
    const NavigationState& current = stack.current();
    assert(current.screen_id == SCREEN_HOME);
    assert(current.selection_index == 0);
    assert(current.scroll_offset == 0);
    printf("PASS: test_initial_state\n");
}

void test_push_and_pop() {
    NavStack stack;
    stack.init();

    NavigationState menu = { SCREEN_MAIN_MENU, 0, 0 };
    bool push_res = stack.push(menu);
    assert(push_res == true);
    assert(stack.get_depth() == 1);
    assert(stack.current().screen_id == SCREEN_MAIN_MENU);
    assert(stack.current().selection_index == 0);

    // Pop back to Home
    bool pop_res = stack.pop();
    assert(pop_res == true);
    assert(stack.get_depth() == 0);
    assert(stack.current().screen_id == SCREEN_HOME);

    // Pop at depth 0 should fail
    bool pop_root_res = stack.pop();
    assert(pop_root_res == false);
    assert(stack.get_depth() == 0);
    assert(stack.current().screen_id == SCREEN_HOME);

    printf("PASS: test_push_and_pop\n");
}

void test_stack_depth_limit() {
    NavStack stack;
    stack.init();

    // Stack has depth 0 initially (1 entry: Home)
    // Can push up to 7 more entries (depth 1..7, total 8 entries)
    for (uint8_t i = 1; i <= 7; i++) {
        NavigationState state = { static_cast<uint8_t>(SCREEN_MAIN_MENU + i), i, 0 };
        assert(stack.push(state) == true);
        assert(stack.get_depth() == i);
        assert(stack.current().screen_id == SCREEN_MAIN_MENU + i);
    }

    // 8th push should fail because MAX_DEPTH = 8
    NavigationState overflow_state = { SCREEN_LOG_WORKOUT_COMPLETE, 99, 0 };
    assert(stack.push(overflow_state) == false);
    assert(stack.get_depth() == 7);

    // Pop all 7 back to 0
    for (int i = 7; i >= 1; i--) {
        assert(stack.pop() == true);
        assert(stack.get_depth() == i - 1);
    }
    assert(stack.current().screen_id == SCREEN_HOME);

    printf("PASS: test_stack_depth_limit\n");
}

void test_selection_persistence_on_pop() {
    NavStack stack;
    stack.init();

    // Push Main Menu
    stack.push({ SCREEN_MAIN_MENU, 0, 0 });
    assert(stack.get_depth() == 1);

    // Change selection index to 2 (Settings)
    stack.set_current_selection(2);
    assert(stack.current().selection_index == 2);

    // Push Settings screen
    stack.push({ SCREEN_SETTINGS_MAIN, 0, 0 });
    assert(stack.get_depth() == 2);
    assert(stack.current().screen_id == SCREEN_SETTINGS_MAIN);

    // Pop back to Main Menu
    assert(stack.pop() == true);
    assert(stack.get_depth() == 1);
    assert(stack.current().screen_id == SCREEN_MAIN_MENU);
    // Selection index on Main Menu should still be 2
    assert(stack.current().selection_index == 2);

    printf("PASS: test_selection_persistence_on_pop\n");
}

void test_reset_to_home() {
    NavStack stack;
    stack.init();

    stack.push({ SCREEN_MAIN_MENU, 1, 0 });
    stack.push({ SCREEN_MUSIC_QUEUE, 0, 0 });
    assert(stack.get_depth() == 2);

    stack.reset_to_home();
    assert(stack.get_depth() == 0);
    assert(stack.current().screen_id == SCREEN_HOME);
    assert(stack.current().selection_index == 0);

    printf("PASS: test_reset_to_home\n");
}

int main() {
    printf("Running NavStack Unit Tests...\n");
    test_initial_state();
    test_push_and_pop();
    test_stack_depth_limit();
    test_selection_persistence_on_pop();
    test_reset_to_home();
    printf("All NavStack tests passed!\n");
    return 0;
}
