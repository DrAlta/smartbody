/****************************************************************************
** Meta object code from reading C++ file 'FaceDialog.h'
**
** Created: Wed Feb 15 11:11:39 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FaceDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FaceDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FaceDialog[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      24,   20,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FaceDialog[] = {
    "FaceDialog\0\0Reset()\0val\0SliderValueChanged(int)\0"
};

const QMetaObject FaceDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_FaceDialog,
      qt_meta_data_FaceDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FaceDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FaceDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FaceDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FaceDialog))
        return static_cast<void*>(const_cast< FaceDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int FaceDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: Reset(); break;
        case 1: SliderValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
