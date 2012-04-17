#include "FaceDialog.h"
#include "vhcl.h"
#include "vhmsg-tt.h"
#include <algorithm>

#define SPIN_BOX "spinBox"

// callbacks
bool GetVisemeNames(void* caller, NetRequest* req);
bool GetAUNumbers(void* caller, NetRequest* req);
//bool GetNumVisemes(void* caller, NetRequest* req);


FaceDialog::FaceDialog(SbmDebuggerClient* client, QWidget* parent ) : QDialog(parent)
{
   m_client = client;
   m_pScene = m_client->GetScene();;
   ui.setupUi(this);
   QVBoxLayout *scrollViewLayout = new QVBoxLayout;
   ui.scrollArea->setLayout(scrollViewLayout);
   ui.scrollArea->widget()->setLayout(scrollViewLayout);

   ui.characterNameBox->clear();
   for (unsigned int i = 0; i < m_pScene->m_characters.size(); i++)
   {
      ui.characterNameBox->addItem(m_pScene->m_characters[i].m_name.c_str());
   }

   connect(ui.resetButton, SIGNAL(pressed()), this, SLOT(Reset()));
   connect(ui.characterNameBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(CharacterSelectionChanged(const QString&)));

   if (m_pScene->m_characters.size() >= 1)
   {
      CharacterSelectionChanged(m_pScene->m_characters[0].m_name.c_str());
   }
}

FaceDialog::~FaceDialog()
{
   
}

void FaceDialog::AddFacialExpression(const string& name, double weight)
{
   weight = std::max<double>(0, weight);
   weight = std::min<double>(1, weight);

   QLabel* expressionName = new QLabel(name.c_str(), this);

   // create slider
   QSlider* weightSlider = new QSlider(this);
   weightSlider->setOrientation(Qt::Horizontal);
   weightSlider->setMaximum(100);
   weightSlider->setMinimum(0);
   weightSlider->setSingleStep(1);
   weightSlider->setValue(weight * 100);
   weightSlider->setObjectName(name.c_str());
   m_Sliders.push_back(weightSlider);

   // create spin box
   QDoubleSpinBox* weightDisplay = new QDoubleSpinBox(this);
   weightDisplay->setEnabled(false);
   weightDisplay->setMaximum(1.0f);
   weightDisplay->setMinimum(0);
   weightDisplay->setValue(weight);
   weightDisplay->setObjectName((name + SPIN_BOX).c_str());

   // create the frame that holds everything
   QFrame* frame = new QFrame(this);
   QHBoxLayout *frameLayout = new QHBoxLayout;
   frameLayout->addWidget(expressionName);
   frameLayout->addWidget(weightSlider);
   frameLayout->addWidget(weightDisplay);
   frame->setLayout(frameLayout);

   // add the frame to the scroll view
   ui.scrollArea->widget()->layout()->addWidget(frame);

   // setup gui messages
   connect(weightSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged(int)));


   // save for later deletion
   m_scrollListChildren.push_back(expressionName);
   m_scrollListChildren.push_back(weightSlider);
   m_scrollListChildren.push_back(weightDisplay);
   m_scrollListChildren.push_back(frame);
}

void FaceDialog::Reset()
{
   for (unsigned int i = 0; i < m_Sliders.size(); i++)
   {
      m_Sliders[i]->setValue(0);
   }
}

void FaceDialog::CharacterSelectionChanged(const QString& selection)
{
   // get AU's
   string commandAU = vhcl::Format("scene.getCharacter(\'%s\').getFaceDefinition().getAUNumbers()", selection.toStdString().c_str());
   m_client->SendSBMCommand(NetRequest::Get_Viseme_Names, "int-array", commandAU, GetAUNumbers, this);

   // get visemes
   string commandViseme = vhcl::Format("scene.getCharacter(\'%s\').getFaceDefinition().getVisemeNames()", selection.toStdString().c_str());
   m_client->SendSBMCommand(NetRequest::Get_Viseme_Names, "string-array", commandViseme, GetVisemeNames, this);

   for (unsigned int i = 0; i < m_scrollListChildren.size(); i++)
   {
      delete m_scrollListChildren[i];
   }
   m_scrollListChildren.clear(); 
   m_Sliders.clear();
}

void FaceDialog::SliderValueChanged(int val)
{
   QSlider* sender = dynamic_cast<QSlider*>(QObject::sender());
   if (!sender)
      return;

   QString visemeName = sender->objectName();

   QDoubleSpinBox* spinBox = ui.scrollArea->findChild<QDoubleSpinBox*>(sender->objectName() + SPIN_BOX);
   if (!spinBox)
      return;

   double weight = (double)val / sender->maximum();
   spinBox->setValue(weight);

   // inform smartbody
   if (visemeName.contains("au"))
   {
      QStringList split = visemeName.split("_");
      if (split.length() >= 3)
      {
         string msg = vhcl::Format("sbm test bml char %s <face type=\"facs\" %s=\"%s\" side=\"%s\" amount=\"%.2f\" />",
            ui.characterNameBox->currentText().toStdString().c_str(), split[0].toStdString().c_str(),
            split[1].toStdString().c_str(), split[2].toStdString().c_str(), weight); 
         vhmsg::ttu_notify1(msg.c_str());
      }
      else if (split.length() == 2)
      {
         string msg = vhcl::Format("sbm test bml char %s <face type=\"facs\" %s=\"%s\" amount=\"%.2f\" />",
            ui.characterNameBox->currentText().toStdString().c_str(), split[0].toStdString().c_str(),
            split[1].toStdString().c_str(), weight); 
         vhmsg::ttu_notify1(msg.c_str());
      }
      else
      {
         printf("Error: failed to manipulate %s", visemeName.toStdString().c_str());
      }
   }
   else
   {
      string msg = vhcl::Format("sbm char %s viseme %s %.2f 1",
         ui.characterNameBox->currentText().toStdString().c_str(), visemeName.toStdString().c_str(), weight); 
      vhmsg::ttu_notify1(msg.c_str());
   }
}

bool GetVisemeNames(void* caller, NetRequest* req)
{
   FaceDialog* dlg = req->getCaller<FaceDialog*>();
   
   vector<string> visemeNames = req->Args();
   for (unsigned int i = 0; i < visemeNames.size(); i++)
   {
      dlg->AddFacialExpression(visemeNames[i].c_str(), 0);
   }

   return true;
}

bool GetAUNumbers(void* caller, NetRequest* req)
{
   FaceDialog* dlg = req->getCaller<FaceDialog*>();
   
   vector<string> auNums = req->Args();
   for (unsigned int i = 0; i < auNums.size(); i++)
   {
      auNums[i].insert(0, "au_");
      dlg->AddFacialExpression(auNums[i].c_str(), 0);
   }

   return true;
}