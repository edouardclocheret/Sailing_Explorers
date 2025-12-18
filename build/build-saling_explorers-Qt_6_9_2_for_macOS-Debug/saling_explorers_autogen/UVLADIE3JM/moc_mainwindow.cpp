/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "onUploadFile",
        "",
        "onSaveImage",
        "onResetBoat",
        "onValChangeP1",
        "newValue",
        "onValChangeP2",
        "onValChangeNearSlider",
        "onValChangeFarSlider",
        "onValChangeNearBox",
        "onValChangeFarBox",
        "onExtraCredit1",
        "onExtraCredit2",
        "onExtraCredit3",
        "onExtraCredit4",
        "onDOFToggled",
        "onDOFFocalChanged",
        "v",
        "onDOFApertureChanged",
        "onDOFMaxBlurChanged",
        "onLSystemToggled",
        "onInstancedForestToggled",
        "onLSystemTypeChanged",
        "idx",
        "onLSystemIterationsChanged",
        "onLSystemScaleChanged",
        "onFogToggled",
        "onFogTypeChanged",
        "onFogDensityChanged",
        "onFogStartChanged",
        "onFogEndChanged",
        "onFogColorChanged",
        "onTreasureCollected",
        "score",
        "onTreasureDirectionChanged",
        "angleDegrees",
        "updateArrowPixmap"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onUploadFile'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSaveImage'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onResetBoat'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onValChangeP1'
        QtMocHelpers::SlotData<void(int)>(5, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onValChangeP2'
        QtMocHelpers::SlotData<void(int)>(7, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onValChangeNearSlider'
        QtMocHelpers::SlotData<void(int)>(8, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onValChangeFarSlider'
        QtMocHelpers::SlotData<void(int)>(9, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onValChangeNearBox'
        QtMocHelpers::SlotData<void(double)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 6 },
        }}),
        // Slot 'onValChangeFarBox'
        QtMocHelpers::SlotData<void(double)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 6 },
        }}),
        // Slot 'onExtraCredit1'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExtraCredit2'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExtraCredit3'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onExtraCredit4'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDOFToggled'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onDOFFocalChanged'
        QtMocHelpers::SlotData<void(double)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 18 },
        }}),
        // Slot 'onDOFApertureChanged'
        QtMocHelpers::SlotData<void(double)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 18 },
        }}),
        // Slot 'onDOFMaxBlurChanged'
        QtMocHelpers::SlotData<void(double)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 18 },
        }}),
        // Slot 'onLSystemToggled'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onInstancedForestToggled'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLSystemTypeChanged'
        QtMocHelpers::SlotData<void(int)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Slot 'onLSystemIterationsChanged'
        QtMocHelpers::SlotData<void(int)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Slot 'onLSystemScaleChanged'
        QtMocHelpers::SlotData<void(double)>(26, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 18 },
        }}),
        // Slot 'onFogToggled'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onFogTypeChanged'
        QtMocHelpers::SlotData<void(int)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 24 },
        }}),
        // Slot 'onFogDensityChanged'
        QtMocHelpers::SlotData<void(double)>(29, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 18 },
        }}),
        // Slot 'onFogStartChanged'
        QtMocHelpers::SlotData<void(double)>(30, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 18 },
        }}),
        // Slot 'onFogEndChanged'
        QtMocHelpers::SlotData<void(double)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Double, 18 },
        }}),
        // Slot 'onFogColorChanged'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTreasureCollected'
        QtMocHelpers::SlotData<void(int)>(33, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 34 },
        }}),
        // Slot 'onTreasureDirectionChanged'
        QtMocHelpers::SlotData<void(float)>(35, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Float, 36 },
        }}),
        // Slot 'updateArrowPixmap'
        QtMocHelpers::SlotData<void(float)>(37, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Float, 36 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onUploadFile(); break;
        case 1: _t->onSaveImage(); break;
        case 2: _t->onResetBoat(); break;
        case 3: _t->onValChangeP1((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->onValChangeP2((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->onValChangeNearSlider((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->onValChangeFarSlider((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->onValChangeNearBox((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 8: _t->onValChangeFarBox((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 9: _t->onExtraCredit1(); break;
        case 10: _t->onExtraCredit2(); break;
        case 11: _t->onExtraCredit3(); break;
        case 12: _t->onExtraCredit4(); break;
        case 13: _t->onDOFToggled(); break;
        case 14: _t->onDOFFocalChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 15: _t->onDOFApertureChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 16: _t->onDOFMaxBlurChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 17: _t->onLSystemToggled(); break;
        case 18: _t->onInstancedForestToggled(); break;
        case 19: _t->onLSystemTypeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->onLSystemIterationsChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 21: _t->onLSystemScaleChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 22: _t->onFogToggled(); break;
        case 23: _t->onFogTypeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 24: _t->onFogDensityChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 25: _t->onFogStartChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 26: _t->onFogEndChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 27: _t->onFogColorChanged(); break;
        case 28: _t->onTreasureCollected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 29: _t->onTreasureDirectionChanged((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 30: _t->updateArrowPixmap((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 31;
    }
    return _id;
}
QT_WARNING_POP
