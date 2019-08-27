#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include "main.h"

using namespace std;

SAMPFUNCS *SF = new SAMPFUNCS();

bool CALLBACK IncomingRPC(stRakNetHookParams *params)
{
	if (params->packetId == RPC_ScrShowDialog)
	{
		WORD dialogId;

		params->bitStream->ResetReadPointer();
		params->bitStream->Read(dialogId);

		// Если это верный диалог то продолжаем читать данные
		if (dialogId == 3385)
		{
			BYTE style, captionLen, button1Len, button2Len;
			char caption[255], button1[25], button2[25], info[4096];

			params->bitStream->Read(style);
			params->bitStream->Read(captionLen);
			params->bitStream->Read(caption, captionLen);
			caption[captionLen] = '\0';
			params->bitStream->Read(button1Len);
			params->bitStream->Read(button1, button1Len);
			button1[button1Len] = '\0';
			params->bitStream->Read(button2Len);
			params->bitStream->Read(button2, button2Len);
			button2[button2Len] = '\0';
			SF->getRakNet()->DecodeString(info, 4096, params->bitStream);

			// Ищем и отправляем ответ
			int first, second;
			if (sscanf(info, "{FFFFFF}На ответ у вас есть {A6F755}7{FFFFFF} секунд!\n\
							%i * %i = { A6F755 } ? {FFFFFF}\n\n\
							Введите результат!", &first, &second))
			{
				string result = to_string(first * second);
				BitStream bsClass;

				bsClass.Write(static_cast<uint16_t>(3385));			// ID диалога
				bsClass.Write(static_cast<uint8_t>(1));				// Нажатая кнопка
				bsClass.Write(static_cast<uint16_t>(65535));		// Айди элемента в списке
				bsClass.Write(static_cast<uint8_t>(result.size()));	// Размерность инпут строки
				bsClass.Write(result);								// Вводимая строка

				SF->getRakNet()->SendRPC(RPC_DialogResponse, &bsClass);
			}
		}
		params->bitStream->ResetReadPointer();
	}
	return true;
}

void CALLBACK mainloop()
{
	static bool initialized = false;
	if (!initialized)
	{
		if (GAME && GAME->GetSystemState() == eSystemState::GS_PLAYING_GAME && SF->getSAMP()->IsInitialized())
		{
			initialized = true;
			SF->getRakNet()->registerRakNetCallback(RakNetScriptHookType::RAKHOOK_TYPE_INCOMING_RPC, IncomingRPC);
			SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(0, 0xAA, 0), "ArizonaMathBot {FFFFFF}by InDigital.");
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
	if (dwReasonForCall == DLL_PROCESS_ATTACH)
		SF->initPlugin(mainloop, hModule);
	return TRUE;
}
