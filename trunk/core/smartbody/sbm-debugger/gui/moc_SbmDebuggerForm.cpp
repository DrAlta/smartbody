/****************************************************************************
** Meta object code from reading C++ file 'SbmDebuggerForm.h'
**
** Created: Wed Feb 15 11:11:39 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SbmDebuggerForm.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SbmDebuggerForm.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SbmDebuggerForm[] = {

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
      17,   16,   16,   16, 0x08,
      37,   16,   16,   16, 0x08,
      58,   16,   16,   16, 0x08,
      79,   16,   16,   16, 0x08,
      99,   16,   16,   16, 0x08,
     122,   16,   16,   16, 0x08,
     139,   16,   16,   16, 0x08,
     156,   16,   16,   16, 0x08,
     186,  169,   16,   16, 0x08,
     268,  242,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SbmDebuggerForm[] = {
    "SbmDebuggerForm\0\0ShowConnectDialog()\0"
    "ShowSettingsDialog()\0ShowResourceDialog()\0"
    "ShowCommandDialog()\0ShowDataViewerDialog()\0"
    "ShowUtilDialog()\0ShowFaceDialog()\0"
    "Disconnect()\0current,previous\0"
    "sceneTreeItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)\0"
    "selectedObj,selectedJoint\0"
    "SetSelectedSceneTreeItem(const Pawn*,const Joint*)\0"
};

const QMetaObject SbmDebuggerForm::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_SbmDebuggerForm,
      qt_meta_data_SbmDebuggerForm, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SbmDebuggerForm::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SbmDebuggerForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SbmDebuggerForm::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SbmDebuggerForm))
        return static_cast<void*>(const_cast< SbmDebuggerForm*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int SbmDebuggerForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ShowConnectDialog(); break;
        case 1: ShowSettingsDialog(); break;
        case 2: ShowResourceDialog(); break;
        case 3: ShowCommandDialog(); break;
        case 4: ShowDataViewerDialog(); break;
        case 5: ShowUtilDialog(); break;
        case 6: ShowFaceDialog(); break;
        case 7: Disconnect(); break;
        case 8: sceneTreeItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 9: SetSelectedSceneTreeItem((*reinterpret_cast< const Pawn*(*)>(_a[1])),(*reinterpret_cast< const Joint*(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 10;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
