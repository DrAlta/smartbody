/****************************************************************************
** Meta object code from reading C++ file 'UtilsDialog.h'
**
** Created: Fri Mar 9 15:55:55 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "UtilsDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UtilsDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_UtilsDialog[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x08,
      29,   12,   12,   12, 0x08,
      45,   12,   12,   12, 0x08,
      63,   12,   12,   12, 0x08,
      84,   12,   12,   12, 0x08,
     104,   12,   12,   12, 0x08,
     118,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_UtilsDialog[] = {
    "UtilsDialog\0\0GazeAtPressed()\0"
    "RunBmlPressed()\0PlayAnimPressed()\0"
    "SpeakButtonPressed()\0QueryAnimsPressed()\0"
    "FilterAnims()\0Refresh()\0"
};

const QMetaObject UtilsDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_UtilsDialog,
      qt_meta_data_UtilsDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &UtilsDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *UtilsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *UtilsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_UtilsDialog))
        return static_cast<void*>(const_cast< UtilsDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int UtilsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: GazeAtPressed(); break;
        case 1: RunBmlPressed(); break;
        case 2: PlayAnimPressed(); break;
        case 3: SpeakButtonPressed(); break;
        case 4: QueryAnimsPressed(); break;
        case 5: FilterAnims(); break;
        case 6: Refresh(); break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
