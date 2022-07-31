/* Edge Impulse ingestion SDK
 * Copyright (c) 2020 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _EDGE_IMPULSE_SDK_REPL_H_
#define _EDGE_IMPULSE_SDK_REPL_H_

#include <string>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <vector>
#include <functional>
#include "thread.hpp"
#include "ticks.hpp"

using namespace cpp_freertos;


using namespace std;

#ifdef ARDUINO
#define SerialStream Stream
#else
#define SerialStream RawSerial
#endif

class ReplBuffer
{
public:
    ReplBuffer()
    {
        position = 0;
    }

    void clear()
    {
        buffer.clear();
        position = 0;
    }

    void add(string s)
    {
        for (string::iterator it = s.begin(); it != s.end(); ++it)
        {
            buffer.insert(buffer.begin() + position, *it);
            position++;
        }
    }

    void add(char c)
    {
        buffer.insert(buffer.begin() + position, c);
        position++;
    }

    vector<char>::iterator begin()
    {
        return buffer.begin();
    }

    vector<char>::iterator end()
    {
        return buffer.end();
    }

    size_t getPosition()
    {
        return position;
    }

    void setPosition(size_t pos)
    {
        position = pos;
    }

    size_t size()
    {
        return buffer.size();
    }

private:
    vector<char> buffer;
    size_t position;
};

class IRepl
{
public:
    virtual void reprintReplState() = 0;
};

static IRepl *replInstance = NULL;

class Repl : public Thread, public IRepl
{
public:
    /**
     * Instantiate a new REPL instance
     *
     * @param tx       UART TX pin (default USBTX)
     * @param rx       UART RX pin (default USBRX)
     * @param baudrate UART baud rate (default MBED_CONF_PLATFORM_STDIO_BAUD_RATE)
     */
    Repl(SerialStream *serial)
        : Thread(1024, 1), pc(serial), historyPosition(0), commandCallback(NULL)
    {
        //pc->baud(MBED_CONF_PLATFORM_STDIO_BAUD_RATE);
        //pc = serial;
        replInstance = this;
    }

    /**
     * Start the REPL
     * This will attach an interrupt to the default serial bus, and will disable deep sleep.
     *
     * @param callback Callback that is invoked when a new command is sent from the REPL
     *                 This takes in a 'const char *' which contains the command just received.
     *                 If your function is synchronous, return 'true' which will reprint the new REPL state.
     *                 If your function is asynchronous, return 'false' and call 'reprintReplState' when the task is done.
     *
     */
    void start(std::function<bool(const char *)> callback)
    {
        reprintReplState();

        commandCallback = callback;
        Start();
    }

    /**
     * Stop the REPL
     * This will detach the interrupt on the default serial bus, and will re-enable deep sleep.
     */
    void stop()
    {
        buffer.clear();

#ifndef ARDUINO
        pc->attach(NULL);
#endif

        commandCallback = NULL;
    }

    /**
     * Prints the current state of the REPL buffer.
     * Invoke this function when a print happens from your userland code, to
     * not confuse your users.
     */
    void reprintReplState()
    {
        string s(buffer.begin(), buffer.end());
        pc->printf("\33[2K\r> %s", s.c_str());
    }

    SerialStream *getSerial()
    {
        return pc;
    }

protected:
    virtual void Run()
    {
        while (true)
        {
            callback_irq();
            Delay(Ticks::MsToTicks(10));
        }
    }

private:
    void rx_callback(char c)
    {
        // control characters start with 0x1b and end with a-zA-Z
        if (inControlChar)
        {

            controlSequence.push_back(c);

            // if a-zA-Z then it's the last one in the control char...
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            {
                inControlChar = false;

                // up
                if (controlSequence.size() == 2 && controlSequence.at(0) == 0x5b && controlSequence.at(1) == 0x41)
                {
                    pc->printf("\033[u"); // restore current position

                    if (historyPosition == 0)
                    {
                        // cannot do...
                    }
                    else
                    {
                        historyPosition--;
                        // reset cursor to 0, do \r, then write the new command...
                        pc->printf("\33[2K\r> %s", history[historyPosition].c_str());

                        buffer.clear();
                        buffer.add(history[historyPosition]);
                    }
                }
                // down
                else if (controlSequence.size() == 2 && controlSequence.at(0) == 0x5b && controlSequence.at(1) == 0x42)
                {
                    pc->printf("\033[u"); // restore current position

                    if (historyPosition == history.size())
                    {
                        // no-op
                    }
                    else if (historyPosition == history.size() - 1)
                    {
                        historyPosition++;

                        // put empty
                        // reset cursor to 0, do \r, then write the new command...
                        pc->printf("\33[2K\r> ");

                        buffer.clear();
                    }
                    else
                    {
                        historyPosition++;
                        // reset cursor to 0, do \r, then write the new command...
                        pc->printf("\33[2K\r> %s", history[historyPosition].c_str());

                        buffer.clear();
                        buffer.add(history[historyPosition]);
                    }
                }
                // left
                else if (controlSequence.size() == 2 && controlSequence.at(0) == 0x5b && controlSequence.at(1) == 0x44)
                {
                    size_t curr = buffer.getPosition();

                    // at pos0? prevent moving to the left
                    if (curr == 0)
                    {
                        pc->printf("\033[u"); // restore current position
                    }
                    // otherwise it's OK, move the cursor back
                    else
                    {
                        buffer.setPosition(curr - 1);

                        pc->print('\033');
                        for (size_t ix = 0; ix < controlSequence.size(); ix++)
                        {
                            pc->write(controlSequence[ix]);
                        }
                    }
                }
                // right
                else if (controlSequence.size() == 2 && controlSequence.at(0) == 0x5b && controlSequence.at(1) == 0x43)
                {
                    size_t curr = buffer.getPosition();
                    size_t size = buffer.size();

                    // already at the end?
                    if (curr == size)
                    {
                        pc->write("\033[u"); // restore current position
                    }
                    else
                    {
                        buffer.setPosition(curr + 1);

                        pc->write('\033');
                        for (size_t ix = 0; ix < controlSequence.size(); ix++)
                        {
                            pc->write(controlSequence[ix]);
                        }
                    }
                }
                else
                {
                    // not up/down? Execute original control sequence
                    pc->write('\033');
                    for (size_t ix = 0; ix < controlSequence.size(); ix++)
                    {
                        pc->write(controlSequence[ix]);
                    }
                }

                controlSequence.clear();
            }
            return;
        }
        switch (c)
        {
        case '\r': /* want to run the buffer */
            pc->write(c);
            pc->write('\n');
            runBuffer();
            break;
        case 0x08: /* backspace */
        case 0x7f: /* also backspace on some terminals */
            handleBackspace();
            break;
        case 0x1b: /* control character */
            // wait until next a-zA-Z
            inControlChar = true;
            pc->printf("\033[s"); // save current position

            break; /* break out of the callback (ignore all other characters) */
        default:
            size_t curr_pos = buffer.getPosition();
            size_t buffer_size = buffer.size();

            if (curr_pos == buffer_size)
            {
                buffer.add(c);
                pc->write(c);
            }
            else
            {
                // super inefficient...
                string v(buffer.begin(), buffer.end());
                v.insert(curr_pos, 1, c);

                buffer.clear();
                buffer.add(v);

                buffer.setPosition(curr_pos + 1);

                pc->printf("\r> %s\033[%dG", v.c_str(), int(curr_pos) + 4);
            }
            break;
        }
    }

    void callback_irq()
    {
        while (ei_get_serial_available() > 0)
        {
            rx_callback(ei_get_serial_byte());
        }

    }

    void handleBackspace()
    {
        size_t curr_pos = buffer.getPosition();

        string v(buffer.begin(), buffer.end());

        if (v.size() == 0 || curr_pos == 0)
            return;

        bool endOfLine = curr_pos == v.size();

        v.erase(curr_pos - 1, 1);

        buffer.clear();
        buffer.add(v);

        buffer.setPosition(curr_pos - 1);

        if (endOfLine)
        {
            pc->printf("\b \b");
        }
        else
        {
            // carriage return, new text, set cursor, empty until end of line
            pc->printf("\r\033[K> %s\033[%dG", v.c_str(), curr_pos + 2);
        }
    }

    void runBuffer()
    {
        string rawCode(buffer.begin(), buffer.end());

        //pc->printf("Running: %s\r\n", rawCode.c_str());

        history.push_back(rawCode);
        historyPosition = history.size();

        // pc->printf("Executing (%s): ", rawCode.c_str());
        // for (size_t ix = 0; ix < rawCode.size(); ix++) {
        //     pc->printf(" %02x ", rawCode.at(ix));
        // }
        // pc->printf("\r\n");

        bool printNewState = true;
        if (commandCallback)
        {
            printNewState = commandCallback(rawCode.c_str());
        }
        buffer.clear();

        if (printNewState)
        {
            pc->printf("> ");
        }
    }

    //RawSerial pc;
    SerialStream *pc;
    ReplBuffer buffer;
    bool inControlChar = false;
    vector<char> controlSequence;
    vector<string> history;
    size_t historyPosition;
    std::function<bool(const char *)> commandCallback;
};

#endif // _EDGE_IMPULSE_SDK_REPL_H_
