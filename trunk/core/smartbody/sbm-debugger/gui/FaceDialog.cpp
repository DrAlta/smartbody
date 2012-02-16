#include "FaceDialog.h"
#include "vhcl.h"
#include "vhmsg-tt.h"
#include <algorithm>

#define SPIN_BOX "spinBox"

FaceDialog::FaceDialog(Scene* pScene, QWidget* parent ) : QDialog(parent)
{
   m_pScene = pScene;
   ui.setupUi(this);
   QVBoxLayout *scrollViewLayout = new QVBoxLayout;
   ui.scrollArea->setLayout(scrollViewLayout);

   AddFacialExpression("au_1_left", 0);
   AddFacialExpression("au_1_right", 0);
   AddFacialExpression("OO", 0);

   ui.characterNameBox->clear();
   for (unsigned int i = 0; i < m_pScene->m_characters.size(); i++)
   {
      ui.characterNameBox->addItem(m_pScene->m_characters[i].m_name.c_str());
   }

   connect(ui.resetButton, SIGNAL(pressed()), this, SLOT(Reset()));
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
   ui.scrollArea->layout()->addWidget(frame);

   // setup gui messages
   connect(weightSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged(int)));
}

void FaceDialog::Reset()
{
   for (unsigned int i = 0; i < m_Sliders.size(); i++)
   {
      m_Sliders[i]->setValue(0);
   }
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