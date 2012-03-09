/****************************************************************************
** Meta object code from reading C++ file 'BMLCreatorDialog.h'
**
** Created: Fri Mar 9 15:55:55 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "BMLCreatorDialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BMLCreatorDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BmlCreatorDialog[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x0a,
      38,   17,   17,   17, 0x0a,
      68,   63,   17,   17, 0x0a,
     102,   63,   17,   17, 0x0a,
     143,  137,   17,   17, 0x0a,
     160,   63,   17,   17, 0x0a,
     186,   17,   17,   17, 0x0a,
     196,   17,   17,   17, 0x0a,
     205,   17,   17,   17, 0x0a,
     224,  216,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_BmlCreatorDialog[] = {
    "BmlCreatorDialog\0\0TypingTextChanged()\0"
    "TypingSelectionChanged()\0text\0"
    "ComboCurrentIndexChanged(QString)\0"
    "CharacterSelectionChanged(QString)\0"
    "value\0SliderMoved(int)\0SpinValueChanged(QString)\0"
    "Refresh()\0RunBml()\0ResetBml()\0currTab\0"
    "ChangedTab(int)\0"
};

const QMetaObject BmlCreatorDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_BmlCreatorDialog,
      qt_meta_data_BmlCreatorDialog, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BmlCreatorDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BmlCreatorDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BmlCreatorDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BmlCreatorDialog))
        return static_cast<void*>(const_cast< BmlCreatorDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int BmlCreatorDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: TypingTextChanged(); break;
        case 1: TypingSelectionChanged(); break;
        case 2: ComboCurrentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: CharacterSelectionChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: SliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: SpinValueChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: Refresh(); break;
        case 7: RunBml(); break;
        case 8: ResetBml(); break;
        case 9: ChangedTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
