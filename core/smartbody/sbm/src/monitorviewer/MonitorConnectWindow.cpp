#include "vhcl.h"
#include "vhmsg-tt.h"

#include "MonitorConnectWindow.h"
#include <sb/SBScene.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBDebuggerClient.h>
#include <sb/SBDebuggerServer.h>
#include <sbm/mcontrol_util.h>
#include <sb/SBPythonClass.h>

MonitorConnectWindow::MonitorConnectWindow(int x, int y, int w, int h, char* label) : Fl_Double_Window(x, y, w, h, label)
{
	set_modal();
	this->label("Monitor Connect");
	this->begin();

	_inputServerName = new Fl_Input(40, 10, 200, 20, "Server");
	_inputServerName->value("localhost");
//	_inputPort = new Fl_Input(40, 40, 200, 20, "Port");
//	_inputPort->value("61616");
//	_inputScope = new Fl_Input(40, 70, 200, 20, "Scope");
//	_inputScope->value("DEFAULT_SCOPE");

	_buttonLoad = new Fl_Button(40, 40, 100, 20, "Load");
	_buttonLoad->callback(OnFefreshCB, this);

	_browserSBProcesses = new Fl_Hold_Browser(10, 70, 300, 200, "Process");
	_buttonOk = new Fl_Button(10, 300, 100, 20, "OK");
	_buttonOk->callback(OnConfirmCB, this);
	_buttonCancel = new Fl_Button(120, 300, 100, 20, "Cancel");
	_buttonCancel->callback(OnCancelCB, this);
	this->end();
}

MonitorConnectWindow::~MonitorConnectWindow()
{
}

void MonitorConnectWindow::show()
{
	Fl_Double_Window::show();
}

void MonitorConnectWindow::hide()
{
	Fl_Double_Window::hide();
}

void MonitorConnectWindow::loadProcesses()
{
	SmartBody::SBScene* sbScene = SmartBody::SBScene::getScene();
	SmartBody::SBScene::getScene()->setRemoteMode(true);

	SBDebuggerClient* c = sbScene->getDebuggerClient();
	SBDebuggerServer* s = sbScene->getDebuggerServer();
	c->QuerySbmProcessIds();
	vhcl::Sleep(2);
	vhmsg::ttu_poll();
	_browserSBProcesses->clear();
	std::vector<std::string> ids = c->GetSbmProcessIds();
	for (size_t i = 0; i < ids.size(); i++)
	{
		if (s->GetID() != ids[i])
			_browserSBProcesses->add(ids[i].c_str());
	}
}

void MonitorConnectWindow::OnConfirmCB(Fl_Widget* widget, void* data)
{
	MonitorConnectWindow* monitorConnectWindow = (MonitorConnectWindow*) data;
	SmartBody::SBScene* sbScene = SmartBody::SBScene::getScene();
	SBDebuggerClient* c = sbScene->getDebuggerClient();
	SBDebuggerServer* s = sbScene->getDebuggerServer();

	if (monitorConnectWindow->_browserSBProcesses->value() <= 0)
		return;

	std::string processId = monitorConnectWindow->_browserSBProcesses->text(monitorConnectWindow->_browserSBProcesses->value());
	if (processId == "")
	{
		fl_alert("Please a process.");
		return;
	}

	c->Disconnect();
	c->Connect(processId);
	vhcl::Sleep(2);
	vhmsg::ttu_poll();
	if (c->GetConnectResult())
	{
		LOG("Connect succeeded to id: %s\n", processId.c_str());
	}
	c->Init();
	c->StartUpdates(sbScene->getSimulationManager()->getTimeDt());
	monitorConnectWindow->hide();
}

void MonitorConnectWindow::OnCancelCB(Fl_Widget* widget, void* data)
{
	MonitorConnectWindow* monitorConnectWindow = (MonitorConnectWindow*) data;
	monitorConnectWindow->hide();
}


void MonitorConnectWindow::OnFefreshCB(Fl_Widget* widget, void* data)
{
	MonitorConnectWindow* monitorConnectWindow = (MonitorConnectWindow*) data;
	
	std::string command = "vhmsgconnect " + std::string(monitorConnectWindow->_inputServerName->value());
	//command = " " + std::string(monitorConnectWindow->_inputPort->value());
	//command = " " + std::string(monitorConnectWindow->_inputPort->value());
	SmartBody::SBScene::getScene()->command(command);
	monitorConnectWindow->loadProcesses();	
}