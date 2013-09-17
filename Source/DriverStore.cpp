#include "../JuceLibraryCode/JuceHeader.h"
#include "DriverStore.h"
#include <wtypes.h>

enum
{
	BLANK_LINE,
	INF_FILE_NAME,
	PROVIDER,
	DEVICE_CLASS,
	DATE_AND_VERSION,
	SIGNER_NAME
};

DriverStore::DriverStore()
{
}

void DriverStore::enumerate()
{
	StringArray providers;
	ChildProcess process;

	providers.add("Broadcom");
	providers.add( "Echo Digital Audio Corporation");

	process.start("pnputil -e");
	String output(process.readAllProcessOutput());
	DBG(output);

	StringArray lines;
	lines.addLines(output);

	int state = BLANK_LINE;
	ScopedPointer<Entry> currentEntry = nullptr;

	for (int i = 0; i < lines.size(); i++)
	{
		switch (state)
		{
			case BLANK_LINE:
				{
					if (lines[i].isEmpty())
					{
						state = INF_FILE_NAME;
						currentEntry = new Entry;
					}
					break;
				}
			case INF_FILE_NAME:
				{
					if(nullptr == currentEntry)
					{
						state = BLANK_LINE;
						break;
					}

					int index = lines[i].indexOfChar(':');

					if (index < 0)
					{
						state = BLANK_LINE;
						break;
					}

					currentEntry->infFileName = lines[i].substring(index + 1).trim();
					state = PROVIDER;
					break;
				}
			case PROVIDER:
				{
					if(nullptr == currentEntry)
					{
						state = BLANK_LINE;
						break;
					}

					int index = lines[i].indexOfChar(':');

					if (index < 0)
					{
						state = BLANK_LINE;
						break;
					}

					currentEntry->provider = lines[i].substring(index + 1).trim();
					state = DEVICE_CLASS;
					break;
				}
			case DEVICE_CLASS:
				{
					if(nullptr == currentEntry)
					{
						state = BLANK_LINE;
						break;
					}

					int index = lines[i].indexOfChar(':');

					if (index < 0)
					{
						state = BLANK_LINE;
						break;
					}

					currentEntry->deviceClass = lines[i].substring(index + 1).trim();
					state = DATE_AND_VERSION;
					break;
				}
			case DATE_AND_VERSION:
				{
					if(nullptr == currentEntry)
					{
						state = BLANK_LINE;
						break;
					}

					int index = lines[i].indexOfChar(':');

					if (index < 0)
					{
						state = BLANK_LINE;
						break;
					}

					currentEntry->dateAndVersion = lines[i].substring(index + 1).trim();
					state = SIGNER_NAME;
					break;
				}
			case SIGNER_NAME:
				{
					if(nullptr == currentEntry)
					{
						state = BLANK_LINE;
						break;
					}

					int index = lines[i].indexOfChar(':');

					if (index < 0)
					{
						state = BLANK_LINE;
						break;
					}

					currentEntry->signerName = lines[i].substring(index + 1).trim();

					if (providers.contains(currentEntry->provider))
					{
						entries.add(currentEntry.release());
					}
					state = BLANK_LINE;
					break;
				}
		}
	}

	for (int i = 0; i < entries.size(); i++)
	{
		DBG(entries[i]->provider);
	}
}

String DriverStore::getINFPath()
{
	String INFPath;
	WCHAR buffer[MAX_PATH];
	GetWindowsDirectoryW(buffer, sizeof(buffer));
	INFPath = buffer;

	INFPath += "\\inf";
	return INFPath;
}

void DriverStore::removeProtocolDrivers()
{
	String const echoProvider("Echo Digital Audio Corporation");
	String const networkProtocol("Network Protocol");
	String INFPath(getINFPath());

	for (int i = 0; i < entries.size(); i++)
	{
		Entry * entry = entries[i];

		if(entry->provider != echoProvider)
		{
			continue;
		}
		if (entry->deviceClass != networkProtocol)
		{
			continue;
		}
		File file(INFPath);
		file = file.getChildFile(entry->infFileName);
		DBG(file.getFullPathName());
		String contents(file.loadFileAsString());

		if (contents.containsIgnoreCase("streamware.sys") || contents.containsIgnoreCase("echobcmeav.sys"))
		{
			ChildProcess process;
			process.start("pnputil -f -d " + entry->infFileName);
			String output(process.readAllProcessOutput());
			DBG(output);

			file.deleteFile();
		}
	}
}

void DriverStore::removeBroadcomDrivers()
{
	String const broadcomProvider("Broadcom");
	String const networkAdapters("Network adapters");
	String INFPath(getINFPath());

	for (int i = 0; i < entries.size(); i++)
	{
		Entry * entry = entries[i];

		if(entry->provider != broadcomProvider)
		{
			continue;
		}

		if (entry->deviceClass != networkAdapters)
		{
			continue;
		}

		File file(INFPath);
		file = file.getChildFile(entry->infFileName);
		DBG(file.getFullPathName());
		String contents(file.loadFileAsString());

		if (contents.containsIgnoreCase("b57nd60x.cat") || contents.containsIgnoreCase("b57nd60a.cat"))
		{
			ChildProcess process;
			process.start("pnputil -f -d " + entry->infFileName);
			String output(process.readAllProcessOutput());
			DBG(output);

			file.deleteFile();
		}
	}
}