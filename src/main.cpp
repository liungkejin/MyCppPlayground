#include <print>
#include "opengl/GLRenderer.h"
#include "face/morph/FaceMorph.h"
#include "utils/EventThread.h"

class Object {
public:
    inline bool no_reference() { return m_ref_ptr.use_count() <= 1; }

    inline int reference_count() { return m_ref_ptr.use_count(); }

private:
    std::shared_ptr<int> m_ref_ptr = std::make_shared<int>();
};

class Test: Object {
public:
    Test(int v) : m_value(v) {}
    Test(const Test & v) : m_value(v.m_value), Object(v) {}

    ~Test() {
        std::cout << m_value <<  ": ref count: " << reference_count() << std::endl;
    }

private:
    int m_value = 100;
};
void testEventThread() {
    wuta::EventThread thread;
    thread.post([]() {
        std::cout << "post run in thread" << std::endl;
    });

    auto eventHandler = [](int e) {
        std::cout << "event handler: " << e << std::endl;
    };
    int id10 = thread.listenEvent(10, eventHandler);
    int id11 = thread.listenEvent(11, eventHandler);

    std::cout << "test EventThread: " << std::endl;
    int code = 0;
    while (code < 1000) {
        std::cin >> code;
        if (code >= 1000) {
            break;
        }
        if (code == 100) {
            thread.removeListener(10, id10);
        } else if (code == 111) {
            thread.removeListener(11, id11);
        }

        thread.post([code]() {
            sleep(1);
            std::cout << "post run: " << code << std::endl;
        });
        thread.send(code);
    }
    thread.quit();
}

// Main code
int main(int argc, char** argv)
{
//    FaceMorphTest::test();
//    testEventThread();
//    {
//        Test t(0);
//        Test b(t);
//        Test c = t;
//        Test d = t;
//    }

    GLRenderer::run();
    return 0;
}
