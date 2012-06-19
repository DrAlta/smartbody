#include "vhcl.h"
#include "vhmsg-tt.h"

#include "MonitorConnectWindow.h"
#include <sb/SBScene.h>
#include <sb/SBSimulationManager.h>
#include <sbm/SbmDebuggerClient.h>
#include <sbm/SbmDebuggerServer.h>

MonitorConnectWindow::MonitorConnectWindow(int x, int y, int w, int h, char* label) : Fl_Double_Window(x, y, w, h, label)
{
	set_modal();
	this->label("Monitor Connect");
	this->begin();
	_browserSBProcesses = new Fl_Hold_Browser(10, 10, 300, 200, "Process");
	_buttonOk = new Fl_Button(10, 250, 100, 20, "OK");
	_buttonOk->callback(OnConfirmCB, this);
	_buttonCancel = new Fl_Button(120, 250, 100, 20, "Cancel");
	_buttonCancel->callback(OnCancelCB, this);
	_buttonRefresh = new Fl_Button(230, 250, 100, 20, "Refresh");
	_buttonRefresh->callback(OnFefreshCB, this);
	this->end();


	loadProcesses();
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

	SbmDebuggerClient* c = sbScene->getDebuggerClient();
	SbmDebuggerServer* s = sbScene->getDebuggerServer();
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
	SbmDebuggerClient* c = sbScene->getDebuggerClient();
	SbmDebuggerServer* s = sbScene->getDebuggerServer();

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
	monitorConnectWindow->loadProcesses();	
}