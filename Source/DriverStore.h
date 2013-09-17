#pragma once

class DriverStore
{
public:
	DriverStore();

	void enumerate();
	String getINFPath();
	void removeProtocolDrivers();
	void removeBroadcomDrivers();
	class Entry
	{
	public:
		String infFileName;
		String provider;
		String deviceClass;
		String dateAndVersion;
		String signerName;
	};
protected:
	OwnedArray<Entry> entries;
private:
};