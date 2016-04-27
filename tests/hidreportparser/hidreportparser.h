/* -*- mode: c++ -*-
 *
 * tests/hidreportparser/hidreportparser.h --
 *
 * Initial software
 * Authors: Izzatbek Mukhanov
 * Copyright © INRIA
 *
 */

#ifndef HID_REPORT_TEST_H
#define HID_REPORT_TEST_H

#include <iostream>
#include <QObject>
#include <QtTest/QtTest>
#include <pointing/utils/HIDReportParser.h>

using namespace pointing;
using namespace std;

class HIDReportTest : public QObject
{
    Q_OBJECT
    HIDReportParser *parser;

private slots:

    // At the beginning
    void initTestCase()
    {
        parser = new HIDReportParser(NULL, 0, 1);
    }

    void simpleMouseEmptyReport()
    {
        // Simple mouse with 2 buttons and wheel, 52 bytes
        unsigned char input[] = {
            0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x09, 0x01, 0xa1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02, 0x75, 0x05, 0x95, 0x01, 0x81, 0x01, 0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x03, 0x81, 0x06, 0xc0, 0xc0
        };
        parser->setDescriptor(input, sizeof(input));
        int dx = 0, dy = 0, buttons = 0;
        int length = parser->getReportLength();
        bool result = parser->getReportData(&dx, &dy, &buttons);
        QCOMPARE(result, true);
        QCOMPARE(length, 4);
        QCOMPARE(dx, 0);
        QCOMPARE(dy, 0);
        QCOMPARE(buttons, 0);
    }

    void simpleMouseReports()
    {
        unsigned char input[] = {
            0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x09, 0x01, 0xa1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02, 0x75, 0x05, 0x95, 0x01, 0x81, 0x01, 0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x03, 0x81, 0x06, 0xc0, 0xc0
        };
        parser->setDescriptor(input, sizeof(input));
        unsigned char report[4] = {0x01, 0x01, 0xFE, 0x00};
        parser->setReport(report);
        int dx = 0, dy = 0, buttons = 0;
        bool result = parser->getReportData(&dx, &dy, &buttons);
        int length = parser->getReportLength();
        QCOMPARE(length, 4);
        QCOMPARE(result, true);
        QCOMPARE(dx, 1);
        QCOMPARE(dy, -2);
        QCOMPARE(buttons, 1);
    }

    void BluetoothM557()
    {
        unsigned char input[] = {
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x02,        // Usage (Mouse)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x02,        //   Report ID (2)
            0x09, 0x01,        //   Usage (Pointer)
            0xA1, 0x00,        //   Collection (Physical)
            0x05, 0x09,        //     Usage Page (Button)
            0x19, 0x01,        //     Usage Minimum (0x01)
            0x29, 0x08,        //     Usage Maximum (0x08)
            0x15, 0x00,        //     Logical Minimum (0)
            0x25, 0x01,        //     Logical Maximum (1)
            0x75, 0x01,        //     Report Size (1)
            0x95, 0x08,        //     Report Count (8)
            0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
            0x09, 0x30,        //     Usage (X)
            0x09, 0x31,        //     Usage (Y)
            0x16, 0x01, 0xF8,  //     Logical Minimum (63489)
            0x26, 0xFF, 0x07,  //     Logical Maximum (2047)
            0x75, 0x0C,        //     Report Size (12)
            0x95, 0x02,        //     Report Count (2)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0x09, 0x38,        //     Usage (Wheel)
            0x15, 0x81,        //     Logical Minimum (129)
            0x25, 0x7F,        //     Logical Maximum (127)
            0x75, 0x08,        //     Report Size (8)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x0C,        //     Usage Page (Consumer)
            0x0A, 0x38, 0x02,  //     Usage (AC Pan)
            0x75, 0x08,        //     Report Size (8)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              //   End Collection
            0xC0,              // End Collection
            0x05, 0x0C,        // Usage Page (Consumer)
            0x09, 0x01,        // Usage (Consumer Control)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x03,        //   Report ID (3)
            0x05, 0x06,        //   Usage Page (Generic Dev Ctrls)
            0x09, 0x20,        //   Usage (Battery Strength)
            0x15, 0x00,        //   Logical Minimum (0)
            0x26, 0x64, 0x00,  //   Logical Maximum (100)
            0x75, 0x08,        //   Report Size (8)
            0x95, 0x01,        //   Report Count (1)
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              // End Collection
            0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
            0x09, 0x01,        // Usage (0x01)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x10,        //   Report ID (16)
            0x75, 0x08,        //   Report Size (8)
            0x95, 0x06,        //   Report Count (6)
            0x15, 0x00,        //   Logical Minimum (0)
            0x26, 0xFF, 0x00,  //   Logical Maximum (255)
            0x09, 0x01,        //   Usage (0x01)
            0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x09, 0x01,        //   Usage (0x01)
            0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
            0xC0,              // End Collection
            0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
            0x09, 0x02,        // Usage (0x02)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x11,        //   Report ID (17)
            0x75, 0x08,        //   Report Size (8)
            0x95, 0x13,        //   Report Count (19)
            0x15, 0x00,        //   Logical Minimum (0)
            0x26, 0xFF, 0x00,  //   Logical Maximum (255)
            0x09, 0x02,        //   Usage (0x02)
            0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x09, 0x02,        //   Usage (0x02)
            0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
            0xC0,              // End Collection
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x06,        // Usage (Keyboard)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x04,        //   Report ID (4)
            0x75, 0x01,        //   Report Size (1)
            0x95, 0x08,        //   Report Count (8)
            0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
            0x19, 0xE0,        //   Usage Minimum (0xE0)
            0x29, 0xE7,        //   Usage Maximum (0xE7)
            0x15, 0x00,        //   Logical Minimum (0)
            0x25, 0x01,        //   Logical Maximum (1)
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x95, 0x01,        //   Report Count (1)
            0x75, 0x08,        //   Report Size (8)
            0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x95, 0x05,        //   Report Count (5)
            0x75, 0x01,        //   Report Size (1)
            0x05, 0x08,        //   Usage Page (LEDs)
            0x19, 0x01,        //   Usage Minimum (Num Lock)
            0x29, 0x05,        //   Usage Maximum (Kana)
            0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
            0x95, 0x01,        //   Report Count (1)
            0x75, 0x03,        //   Report Size (3)
            0x91, 0x03,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
            0x95, 0x06,        //   Report Count (6)
            0x75, 0x08,        //   Report Size (8)
            0x15, 0x00,        //   Logical Minimum (0)
            0x26, 0xFF, 0x00,  //   Logical Maximum (255)
            0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
            0x19, 0x00,        //   Usage Minimum (0x00)
            0x29, 0xFF,        //   Usage Maximum (0xFF)
            0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              // End Collection
            0x05, 0x0C,        // Usage Page (Consumer)
            0x09, 0x01,        // Usage (Consumer Control)
            0xA1, 0x01,        // Collection (Application)
            0x85, 0x05,        //   Report ID (5)
            0x15, 0x00,        //   Logical Minimum (0)
            0x25, 0x01,        //   Logical Maximum (1)
            0x75, 0x01,        //   Report Size (1)
            0x95, 0x02,        //   Report Count (2)
            0x0A, 0x25, 0x02,  //   Usage (AC Forward)
            0x0A, 0x24, 0x02,  //   Usage (AC Back)
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x95, 0x01,        //   Report Count (1)
            0x75, 0x06,        //   Report Size (6)
            0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0xC0              // End Collection
        };
        // 246 bytes

        parser->setDescriptor(input, sizeof(input));
        unsigned char falseReport = 255;
        bool result = parser->setReport(&falseReport);
        QCOMPARE(result, false);
        int length = parser->getReportLength();
        QCOMPARE(length, 7);
        unsigned char report[7] = { 0x02, 0x01, 0xFF, 0x03, 0x02, 0x00, 0x00 };
        parser->setReport(report);
        int dx = 0, dy = 0, buttons = 0;
        parser->getReportData(&dx, &dy, &buttons);
        QCOMPARE(dx, 1023);
        QCOMPARE(dy, 32);
        QCOMPARE(buttons, 1);
    }

    void GamingMouseG502()
    {
        unsigned char input[] = {
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x02,        // Usage (Mouse)
            0xA1, 0x01,        // Collection (Application)
            0x09, 0x01,        //   Usage (Pointer)
            0xA1, 0x00,        //   Collection (Physical)
            0x05, 0x09,        //     Usage Page (Button)
            0x19, 0x01,        //     Usage Minimum (0x01)
            0x29, 0x10,        //     Usage Maximum (0x10)
            0x15, 0x00,        //     Logical Minimum (0)
            0x25, 0x01,        //     Logical Maximum (1)
            0x95, 0x10,        //     Report Count (16)
            0x75, 0x01,        //     Report Size (1)
            0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
            0x16, 0x01, 0x80,  //     Logical Minimum (32769)
            0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
            0x75, 0x10,        //     Report Size (16)
            0x95, 0x02,        //     Report Count (2)
            0x09, 0x30,        //     Usage (X)
            0x09, 0x31,        //     Usage (Y)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0x15, 0x81,        //     Logical Minimum (129)
            0x25, 0x7F,        //     Logical Maximum (127)
            0x75, 0x08,        //     Report Size (8)
            0x95, 0x01,        //     Report Count (1)
            0x09, 0x38,        //     Usage (Wheel)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x0C,        //     Usage Page (Consumer)
            0x0A, 0x38, 0x02,  //     Usage (AC Pan)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              //   End Collection
            0xC0               // End Collection

            // 67 bytes

            // best guess: USB HID Report Descriptor
        };
        parser->setDescriptor(input, sizeof(input));
        unsigned char report[8] = { 0x01, 0x00, 0x05, 0x00, 0x00, 0x05, 0x00, 0x00 };
        parser->setReport(report);
        int length = parser->getReportLength();
        QCOMPARE(length, 8);
        int dx = 0, dy = 0, buttons = 0;
        parser->getReportData(&dx, &dy, &buttons);
        QCOMPARE(dx, 5);
        QCOMPARE(dy, 1280);
        QCOMPARE(buttons, 1);
    }

    void mx310()
    {
        unsigned char input[] = {
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x02,        // Usage (Mouse)
            0xA1, 0x01,        // Collection (Application)
            0x09, 0x01,        //   Usage (Pointer)
            0xA1, 0x00,        //   Collection (Physical)
            0x05, 0x09,        //     Usage Page (Button)
            0x19, 0x01,        //     Usage Minimum (0x01)
            0x29, 0x06,        //     Usage Maximum (0x06)
            0x15, 0x00,        //     Logical Minimum (0)
            0x25, 0x01,        //     Logical Maximum (1)
            0x95, 0x06,        //     Report Count (6)
            0x75, 0x01,        //     Report Size (1)
            0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x95, 0x02,        //     Report Count (2)
            0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
            0x09, 0x30,        //     Usage (X)
            0x09, 0x31,        //     Usage (Y)
            0x09, 0x38,        //     Usage (Wheel)
            0x15, 0x81,        //     Logical Minimum (129)
            0x25, 0x7F,        //     Logical Maximum (127)
            0x75, 0x08,        //     Report Size (8)
            0x95, 0x03,        //     Report Count (3)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              //   End Collection
            0x05, 0x08,        //   Usage Page (LEDs)
            0x09, 0x4B,        //   Usage (Generic Indicator)
            0x95, 0x08,        //   Report Count (8)
            0x75, 0x01,        //   Report Size (1)
            0x15, 0x00,        //   Logical Minimum (0)
            0x25, 0x01,        //   Logical Maximum (1)
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              // End Collection

            // 64 bytes
        };
        parser->setDescriptor(input, sizeof(input));
        unsigned char report[5] = { 0x01, 0x05, 0x05, 0x00, 0x00 };
        parser->setReport(report);
        int length = parser->getReportLength();
        QCOMPARE(length, 5);
        int dx = 0, dy = 0, buttons = 0;
        parser->getReportData(&dx, &dy, &buttons);
        QCOMPARE(dx, 5);
        QCOMPARE(dy, 5);
        QCOMPARE(buttons, 1);
    }

    void dell()
    {
        unsigned char input[] = {
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x02,        // Usage (Mouse)
            0xA1, 0x01,        // Collection (Application)
            0x09, 0x01,        //   Usage (Pointer)
            0xA1, 0x00,        //   Collection (Physical)
            0x05, 0x09,        //     Usage Page (Button)
            0x19, 0x01,        //     Usage Minimum (0x01)
            0x29, 0x05,        //     Usage Maximum (0x05)
            0x15, 0x00,        //     Logical Minimum (0)
            0x25, 0x01,        //     Logical Maximum (1)
            0x95, 0x05,        //     Report Count (5)
            0x75, 0x01,        //     Report Size (1)
            0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x95, 0x01,        //     Report Count (1)
            0x75, 0x03,        //     Report Size (3)
            0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
            0x09, 0x30,        //     Usage (X)
            0x09, 0x31,        //     Usage (Y)
            0x16, 0x01, 0xF8,  //     Logical Minimum (63489)
            0x26, 0xFF, 0x07,  //     Logical Maximum (2047)
            0x75, 0x0C,        //     Report Size (12)
            0x95, 0x02,        //     Report Count (2)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0x09, 0x38,        //     Usage (Wheel)
            0x15, 0x81,        //     Logical Minimum (129)
            0x25, 0x7F,        //     Logical Maximum (127)
            0x75, 0x08,        //     Report Size (8)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              //   End Collection
            0xC0               // End Collection

            // 64 bytes

        };
        parser->setDescriptor(input, sizeof(input));
        unsigned char report[5] = { 0xFF, 0x05, 0x05, 0x05, 0x00 };
        parser->setReport(report);
        int length = parser->getReportLength();
        QCOMPARE(length, 5);
        int dx = 0, dy = 0, buttons = 0;
        parser->getReportData(&dx, &dy, &buttons);
        QCOMPARE(dx, 1285);
        QCOMPARE(dy, 80);
        QCOMPARE(buttons, 7);
    }

    void logitechM235()
    {
        unsigned char input[] = {
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x02,        // Usage (Mouse)
            0xA1, 0x01,        // Collection (Application)
            0x09, 0x01,        //   Usage (Pointer)
            0xA1, 0x00,        //   Collection (Physical)
            0x05, 0x09,        //     Usage Page (Button)
            0x19, 0x01,        //     Usage Minimum (0x01)
            0x29, 0x10,        //     Usage Maximum (0x10)
            0x15, 0x00,        //     Logical Minimum (0)
            0x25, 0x01,        //     Logical Maximum (1)
            0x95, 0x10,        //     Report Count (16)
            0x75, 0x01,        //     Report Size (1)
            0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
            0x16, 0x01, 0x80,  //     Logical Minimum (32769)
            0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
            0x75, 0x10,        //     Report Size (16)
            0x95, 0x02,        //     Report Count (2)
            0x09, 0x30,        //     Usage (X)
            0x09, 0x31,        //     Usage (Y)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0x15, 0x81,        //     Logical Minimum (129)
            0x25, 0x7F,        //     Logical Maximum (127)
            0x75, 0x08,        //     Report Size (8)
            0x95, 0x01,        //     Report Count (1)
            0x09, 0x38,        //     Usage (Wheel)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x0C,        //     Usage Page (Consumer)
            0x0A, 0x38, 0x02,  //     Usage (AC Pan)
            0x95, 0x01,        //     Report Count (1)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              //   End Collection
            0xC0,              // End Collection

            // 67 bytes
        };
        parser->setDescriptor(input, sizeof(input));
        unsigned char report[8] = { 0x03, 0x00, 0x05, 0x05, 0x05, 0x00, 0x00, 0x00 };
        parser->setReport(report);
        int length = parser->getReportLength();
        QCOMPARE(length, 8);
        int dx = 0, dy = 0, buttons = 0;
        parser->getReportData(&dx, &dy, &buttons);
        QCOMPARE(dx, 1285);
        QCOMPARE(dy, 5);
        QCOMPARE(buttons, 3);
    }

    void appleTouchpad()
    {
        unsigned char input[] = {
            0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
            0x09, 0x02,        // Usage (Mouse)
            0xA1, 0x01,        // Collection (Application)
            0x05, 0x09,        //   Usage Page (Button)
            0x19, 0x01,        //   Usage Minimum (0x01)
            0x29, 0x01,        //   Usage Maximum (0x01)
            0x15, 0x00,        //   Logical Minimum (0)
            0x25, 0x01,        //   Logical Maximum (1)
            0x95, 0x01,        //   Report Count (1)
            0x75, 0x01,        //   Report Size (1)
            0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x95, 0x07,        //   Report Count (7)
            0x75, 0x01,        //   Report Size (1)
            0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
            0x09, 0x01,        //   Usage (Pointer)
            0xA1, 0x00,        //   Collection (Physical)
            0x09, 0x30,        //     Usage (X)
            0x09, 0x31,        //     Usage (Y)
            0x16, 0x01, 0x80,  //     Logical Minimum (32769)
            0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
            0x36, 0x00, 0x00,  //     Physical Minimum (0)
            0x46, 0x00, 0x00,  //     Physical Maximum (0)
            0x55, 0x00,        //     Unit Exponent (0)
            0x65, 0x00,        //     Unit (None)
            0x75, 0x10,        //     Report Size (16)
            0x95, 0x02,        //     Report Count (2)
            0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              //   End Collection
            0x09, 0x38,        //   Usage (Wheel)
            0x15, 0x81,        //   Logical Minimum (129)
            0x25, 0x7F,        //   Logical Maximum (127)
            0x35, 0x00,        //   Physical Minimum (0)
            0x45, 0x00,        //   Physical Maximum (0)
            0x55, 0x00,        //   Unit Exponent (0)
            0x65, 0x00,        //   Unit (None)
            0x75, 0x08,        //   Report Size (8)
            0x95, 0x01,        //   Report Count (1)
            0x81, 0x06,        //   Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,              // End Collection

            // 82 bytes
        };
        parser->setDescriptor(input, sizeof(input));
        unsigned char report[6] = { 0x01, 0x05, 0x05, 0x05, 0x00, 0x00 };
        parser->setReport(report);
        int length = parser->getReportLength();
        QCOMPARE(length, 6);
        int dx = 0, dy = 0, buttons = 0;
        parser->getReportData(&dx, &dy, &buttons);
        QCOMPARE(dx, 1285);
        QCOMPARE(dy, 5);
        QCOMPARE(buttons, 1);
    }

    void microsoftNano()
    {
      unsigned char input[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x02,        // Usage (Mouse)
        0xA1, 0x01,        // Collection (Application)
        0x05, 0x09,        //   Usage Page (Button)
        0x19, 0x01,        //   Usage Minimum (0x01)
        0x29, 0x01,        //   Usage Maximum (0x01)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x01,        //   Report Size (1)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x07,        //   Report Count (7)
        0x75, 0x01,        //   Report Size (1)
        0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
        0x09, 0x01,        //   Usage (Pointer)
        0xA1, 0x00,        //   Collection (Physical)
        0x09, 0x30,        //     Usage (X)
        0x09, 0x31,        //     Usage (Y)
        0x16, 0x01, 0x80,  //     Logical Minimum (32769)
        0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
        0x36, 0x00, 0x00,  //     Physical Minimum (0)
        0x46, 0x00, 0x00,  //     Physical Maximum (0)
        0x55, 0x00,        //     Unit Exponent (0)
        0x65, 0x00,        //     Unit (None)
        0x75, 0x10,        //     Report Size (16)
        0x95, 0x02,        //     Report Count (2)
        0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //   End Collection
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0x00,              //   Unknown (bTag: 0x00, bType: 0x00)
        0xC0,              // End Collection

        // 82 bytes
      };
      parser->setDescriptor(input, sizeof(input));
      unsigned char report[5] = { 0x01, 0x05, 0x00, 0x00, 0x00 };
      parser->setReport(report);
      int length = parser->getReportLength();
      QCOMPARE(length, 5);
      int dx = 0, dy = 0, buttons = 0;
      parser->getReportData(&dx, &dy, &buttons);
      QCOMPARE(dx, 5);
      QCOMPARE(dy, 0);
      QCOMPARE(buttons, 1);
    }

    void iFeelMouse()
    {
      unsigned char input[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x02,        // Usage (Mouse)
        0xA1, 0x01,        // Collection (Application)
        0xA1, 0x02,        //   Collection (Logical)
        0x05, 0x09,        //     Usage Page (Button)
        0x19, 0x01,        //     Usage Minimum (0x01)
        0x29, 0x03,        //     Usage Maximum (0x03)
        0x15, 0x00,        //     Logical Minimum (0)
        0x25, 0x01,        //     Logical Maximum (1)
        0x75, 0x01,        //     Report Size (1)
        0x95, 0x03,        //     Report Count (3)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x05,        //     Report Count (5)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x09, 0x01,        //     Usage (0x01)
        0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
        0x09, 0x01,        //     Usage (Pointer)
        0xA1, 0x00,        //     Collection (Physical)
        0x15, 0x81,        //       Logical Minimum (129)
        0x25, 0x7F,        //       Logical Maximum (127)
        0x75, 0x08,        //       Report Size (8)
        0x95, 0x02,        //       Report Count (2)
        0x09, 0x30,        //       Usage (X)
        0x09, 0x31,        //       Usage (Y)
        0x81, 0x06,        //       Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //     End Collection
        0x09, 0x38,        //     Usage (Wheel)
        0x95, 0x01,        //     Report Count (1)
        0x81, 0x06,        //     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              //   End Collection
        0xA1, 0x02,        //   Collection (Logical)
        0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
        0x09, 0x02,        //     Usage (0x02)
        0x95, 0x07,        //     Report Count (7)
        0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,              //   End Collection
        0xC0,              // End Collection

        // 74 bytes
      };
      parser->setDescriptor(input, sizeof(input));
      unsigned char report[4] = { 0x01, 0x05, 0x05, 0x00};
      parser->setReport(report);
      int length = parser->getReportLength();
      QCOMPARE(length, 4);
      int dx = 0, dy = 0, buttons = 0;
      parser->getReportData(&dx, &dy, &buttons);
      QCOMPARE(dx, 5);
      QCOMPARE(dy, 5);
      QCOMPARE(buttons, 1);
    }

    // At the end
    void cleanupTestCase()
    {
        delete parser;
    }
};

#endif // HID_REPORT_TEST_H

QTEST_MAIN(HIDReportTest)
