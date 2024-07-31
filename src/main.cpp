#include "opengl/GLRenderer.h"
#include "face/morph/FaceMorph.h"

#include "eventpp/eventqueue.h"

void testEventQueue() {
    eventpp::EventQueue<int, void (const std::string &, const bool)> queue;

    queue.appendListener(3, [](const std::string &s, bool b) {
        std::cout << std::boolalpha << "Got event 3, s is " << s << " b is " << b << std::endl;
    });
    queue.appendListener(5, [](const std::string &s, bool b) {
        std::cout << std::boolalpha << "Got event 5, s is " << s << " b is " << b << std::endl;
    });

// The listeners are not triggered during enqueue.

// Process the event queue, dispatch all queued events.
    int code = 0;
    while (code < 10) {
        std::cin >> code;

        queue.enqueue(code, "Hello", true);

        queue.process();
    }
}

// Main code
int main(int argc, char** argv)
{
//    FaceMorphTest::test();
    testEventQueue();

//    GLRenderer::run();
    return 0;
}
