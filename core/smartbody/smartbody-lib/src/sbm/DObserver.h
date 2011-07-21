#ifndef DOBSERVER_H
#define DOBSERVER_H

class DSubject;

class DObserver
{
	public:
		DObserver();
		~DObserver();

		virtual void notify(DSubject* subject) = 0;
};

#endif