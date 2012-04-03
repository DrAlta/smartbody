/****************************************************************************
** Meta object code from reading C++ file 'SbmDebuggerForm.h'
**
** Created: Fri Mar 16 17:54:30 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SbmDebuggerForm.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SbmDebuggerForm.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SbmDebuggerForm[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
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
     179,   16,   16,   16, 0x08,
     205,   16,   16,   16, 0x08,
     235,  218,   16,   16, 0x08,
     317,  291,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SbmDebuggerForm[] = {
    "SbmDebuggerForm\0\0ShowConnectDialog()\0"
    "ShowSettingsDialog()\0ShowResourceDialog()\0"
    "ShowCommandDialog()\0ShowDataViewerDialog()\0"
    "ShowUtilDialog()\0ShowFaceDialog()\0"
    "ShowBmlCreatorDialog()\0ShowEntityCreatorDialog()\0"
    "Disconnect()\0current,previous\0"
    "sceneTreeItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)\0"
    "selectedObj,selectedJoint\0"
    "SetSelectedSceneTreeItem(const Pawn*,const Joint*)\0"
};

void SbmDebuggerForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SbmDebuggerForm *_t = static_cast<SbmDebuggerForm *>(_o);
        switch (_id) {
        case 0: _t->ShowConnectDialog(); break;
        case 1: _t->ShowSettingsDialog(); break;
        case 2: _t->ShowResourceDialog(); break;
        case 3: _t->ShowCommandDialog(); break;
        case 4: _t->ShowDataViewerDialog(); break;
        case 5: _t->ShowUtilDialog(); break;
        case 6: _t->ShowFaceDialog(); break;
        case 7: _t->ShowBmlCreatorDialog(); break;
        case 8: _t->ShowEntityCreatorDialog(); break;
        case 9: _t->Disconnect(); break;
        case 10: _t->sceneTreeItemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< QTreeWidgetItem*(*)>(_a[2]))); break;
        case 11: _t->SetSelectedSceneTreeItem((*reinterpret_cast< const Pawn*(*)>(_a[1])),(*reinterpret_cast< const Joint*(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SbmDebuggerForm::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SbmDebuggerForm::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_SbmDebuggerForm,
      qt_meta_data_SbmDebuggerForm, &staticMetaObjectExtraData }
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
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
