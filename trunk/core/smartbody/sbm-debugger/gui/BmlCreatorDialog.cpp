#include "BmlCreatorDialog.h"
#include "vhcl.h"
#include "vhmsg-tt.h"

BmlCreatorDialog::BmlCreatorDialog(SbmDebuggerClient* client, QWidget *parent)
{
   m_client = client;
   m_pScene = client->GetScene();
   ui.setupUi(this);

   for (int i = 0; i < children().length(); i++)
   {
      BuildConnections(children()[i]);
   }

   // we don't want these signals sent for the main typing box that sends the bml
   disconnect(ui.bmlCommandBox, SIGNAL(textChanged()), this, SLOT(TypingTextChanged()));
   disconnect(ui.bmlCommandBox, SIGNAL(selectionChanged()), this, SLOT(TypingSelectionChanged()));
   disconnect(ui.characterBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(ComboCurrentIndexChanged(const QString&)));

   connect(ui.refreshButton, SIGNAL(pressed()), this, SLOT(Refresh()));
   connect(ui.resetBml, SIGNAL(pressed()), this, SLOT(ResetBml()));
   connect(ui.runButton, SIGNAL(pressed()), this, SLOT(RunBml()));
   connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(ChangedTab(int)));
   connect(ui.characterBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(CharacterSelectionChanged(const QString&)));

   Refresh();
}

BmlCreatorDialog::~BmlCreatorDialog()
{
   
}  

void BmlCreatorDialog::BuildConnections(QObject* widget)
{
   if (dynamic_cast<QPlainTextEdit*>(widget))
   {
      connect(widget, SIGNAL(textChanged()), this, SLOT(TypingTextChanged()));
      connect(widget, SIGNAL(selectionChanged()), this, SLOT(TypingSelectionChanged()));
   }
   else if (dynamic_cast<QComboBox*>(widget))
   {
      connect(widget, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(ComboCurrentIndexChanged(const QString&)));
   }
   else if (dynamic_cast<QSlider*>(widget))
   {
      connect(widget, SIGNAL(sliderMoved(int)), this, SLOT(SliderMoved(int)));
   }
   else if (dynamic_cast<QDoubleSpinBox*>(widget))
   {
      connect(widget, SIGNAL(valueChanged(const QString&)), this, SLOT(SpinValueChanged(const QString&)));
   }

   for (int i = 0; i < widget->children().length(); i++)
   {
      BuildConnections(widget->children()[i]);
   }
}

void BmlCreatorDialog::TypingTextChanged()
{
   QPlainTextEdit* sender = dynamic_cast<QPlainTextEdit*>(QObject::sender());
   if (!sender)
      return;

   AppendBml(sender->accessibleName(), sender->toPlainText());
}

void BmlCreatorDialog::TypingSelectionChanged()
{
   QPlainTextEdit* sender = dynamic_cast<QPlainTextEdit*>(QObject::sender());
   if (!sender)
      return;

   if (sender->toPlainText().length() > 0)
   {
      AppendBml(sender->accessibleName(), sender->toPlainText());
   }
}

void BmlCreatorDialog::ComboCurrentIndexChanged(const QString & text)
{
   QComboBox* sender = dynamic_cast<QComboBox*>(QObject::sender());
   if (!sender)
      return;

   AppendBml(sender->accessibleName(), text);
}

void BmlCreatorDialog::CharacterSelectionChanged(const QString & text)
{
   QString bml = ui.bmlCommandBox->toPlainText();
   int index = bml.indexOf("char");
   if (index > -1)
   {
      index += 5; // get passed the string "char "
      int bmlStart = bml.indexOf(" ", index);
      if (bmlStart > -1)
      {
         bml = bml.remove(index, bmlStart - index);
         bml.insert(index, text);
      }
      ui.bmlCommandBox->setPlainText(bml);
   }
}

void BmlCreatorDialog::SliderMoved(int value)
{
   QSlider* sender = dynamic_cast<QSlider*>(QObject::sender());
   if (!sender)
      return;

   char text[8] = {0};
   _itoa_s(value, text, sizeof(text), 10);
   AppendBml(sender->accessibleName(), text);
}

void BmlCreatorDialog::SpinValueChanged(const QString & text)
{
   QDoubleSpinBox* sender = dynamic_cast<QDoubleSpinBox*>(QObject::sender());
   if (!sender)
      return;

   AppendBml(sender->accessibleName(), text);
} 

QString BmlCreatorDialog::GetSelectedChar()
{
   return ui.characterBox->currentText();
}

void BmlCreatorDialog::Refresh()
{
   ui.characterBox->clear();
   for (unsigned int i = 0; i < m_pScene->m_characters.size(); i++)
   {
      ui.characterBox->addItem(m_pScene->m_characters[i].m_name.c_str());
   }
}

void BmlCreatorDialog::RunBml()
{
   vhmsg::ttu_notify2("sbm", ui.bmlCommandBox->toPlainText().toStdString().c_str());
}

void BmlCreatorDialog::ResetBml()
{
   
}

void BmlCreatorDialog::ChangedTab(int currTab)
{
   ui.bmlCommandBox->clear();
}

void BmlCreatorDialog::AppendBml(const QString& name, const QString& attribute)
{
   QString currentCommand = ui.bmlCommandBox->toPlainText();
   QString textToAppend = "";
   int closingSymbolIndex = currentCommand.lastIndexOf('/');

   if (currentCommand.contains("bml char") && closingSymbolIndex != -1)
   {
      int nameIndex = currentCommand.indexOf(name);
      if (nameIndex > -1)
      {
         // command already exists, update it 
         int attrStart = nameIndex + (name.length() + 2); // get passed the name="
         int attrEndQuote = currentCommand.indexOf("\"", attrStart);

         if (attribute.length() > 0)
         {
            if (attrEndQuote > -1)
            {
               // update with new attribute data
               currentCommand = currentCommand.remove(attrStart, attrEndQuote - attrStart);
               textToAppend = currentCommand.insert(attrStart, attribute);
            }
         }
         else
         {
            // get rid of attribute
            attrEndQuote += 2; // take care of white space
            textToAppend = currentCommand.remove(nameIndex - 1, attrEndQuote - nameIndex);
         }
      }
      else
      {
         if (attribute.length() == 0)
            return;

         // command doesn't exist, add it
         textToAppend = QString(" %1=\"%2\"").arg(name).arg(attribute);
         textToAppend = currentCommand.insert(closingSymbolIndex, textToAppend);     
      }
   }
   else
   {
      // fresh start
      textToAppend = QString("bml char %1 <%2 %3=\"%4\"/>").arg(GetSelectedChar()).arg(ui.tabWidget->tabText(ui.tabWidget->currentIndex())).
         arg(name).arg(attribute);
   } 

   ui.bmlCommandBox->setPlainText(textToAppend);
}