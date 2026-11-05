#include "network.h"
#include "sprite.h"
#include "unsorted.h"

// The outgoing frame and session-setup packets (0x4b409c, 0x4b40a8).
extern NetPacket g_NetInputPacket;
extern NetPacket g_NetSetupPacket;

int App::UpdateNetPlay()
{
    int result = 0;
    int status;
    DWORD now;
    WORD seeds[2];
    int predicted;
    WORD inputs[2];
    Network *network;

    this->EnterCriticalSection(4);
    if (g_Network->unknownB8 == 0)
    {
        if (g_Network->side != 0)
        {
            g_Network->unknownAc = 1;
            g_Network->CreateDeviceAddress();
            g_Network->CreateHostAddress();
            g_Network->Connect();
            this->connectStartTime = timeGetTime();
            g_Network->unknownB8 = 1;
            this->unknown478 = 1;
            g_SpriteEngine->RenderTextCentered(this->unknown584, 0xffffff, 0, "接続可能な妖怪を探しています");
        }
        else
        {
        host:
            Sleep(100);
            g_Network->unknownAc = 0;
            g_Network->CreateDeviceAddress();
            g_Network->Host();
            g_SpriteEngine->RenderTextCentered(this->unknown584, 0xffffff, 0, "妖怪の接続を待っています");
            g_Network->unknownB8 = 2;
            this->unknown474 = timeGetTime();
        }
    }
    if (g_Network->unknownB8 == 1)
    {
        now = timeGetTime();
        this->unknown478 = 1;
        if (now < this->connectStartTime)
        {
            this->connectStartTime = now;
        }
        if (now - this->connectStartTime >= g_Rng.GetRandomU32InRange(4000) + 4000 && g_Network->side != 1)
        {
            g_Network->Close();
            goto host;
        }
        goto wait;
    }
    if (g_Network->unknownB8 == 2 || g_Network->unknownB8 == 3)
    {
        if (g_Network->GetOtherUnknown1c() == 0)
        {
            if (this->unknown458 != 0)
            {
                status = 3;
                goto end;
            }
            this->unknown478 = 1;
            now = timeGetTime();
            if (now < this->unknown474)
            {
                this->unknown474 = now;
            }
            if (now - this->unknown474 > g_Rng.GetRandomU32InRange(4000) + 6000)
            {
                g_Network->Close();
                g_Network->unknownB8 = 0;
                this->unknown474 = timeGetTime();
            }
            goto wait;
        }
        this->unknown478 = 0;
        this->unknown474 = timeGetTime();
    }
    network = g_Network;
    if (this->unknown460 == 0 || this->unknown460 == 2)
    {
        if (this->unknown458 == 0 || (unsigned int)this->unknown458 % (60 / g_Network->syncFrames) == 0)
        {
            if (this->unknown460 == 0)
            {
                if (network->initialFramesQueued == 0)
                {
                    if (network->unknownAc == 0 && 60 / network->syncFrames - 1 > 0)
                    {
                        for (int frame = -9998; frame + 9998 < 60 / network->syncFrames - 1; frame++)
                        {
                            g_App.AddRecord(0, frame, 0, g_Rng.GetSeed());
                            g_App.AddRecord(1, frame, 0, g_Rng.GetSeed());
                        }
                    }
                    network->unknownB0 = 0;
                    Network *sender = g_Network;
                    g_NetSetupPacket.type = 2;
                    g_NetSetupPacket.side = (char)g_Network->unknownAc;
                    g_NetSetupPacket.syncFrames = g_Network->syncFrames;
                    g_NetSetupPacket.unknown8 = g_Network->isLan;
                    g_NetSetupPacket.seed = g_Rng.GetSeed();
                    sender->Send(&g_NetSetupPacket, sizeof(g_NetSetupPacket));
                    if (g_Network->isLan != 0)
                    {
                        this->PopRecord(0, &seeds[0], &predicted);
                        this->PopRecord(1, &seeds[1], &predicted);
                    }
                    g_Network->initialFramesQueued = 1;
                }
                if (g_Network->unknownB0 == 0)
                {
                waitForFrame:
                    result = 1;
                    this->LeaveCriticalSection(4);
                    return result;
                }
                g_NetInputPacket.input = ReadNetInput();
                this->unknown458++;
                this->unknown464 = timeGetTime();
            }
            this->unknown468 = timeGetTime();
            g_Network->unknownB0 = 0;
            Network *sender = g_Network;
            g_NetInputPacket.type = 0;
            char side = (char)g_Network->unknownAc;
            g_NetInputPacket.side = side;
            g_NetInputPacket.value = 60 / g_Network->syncFrames - 1 + this->unknown458;
            g_NetInputPacket.seed = g_Rng.GetSeed();
            g_NetInputPacket.checksum = side + g_NetInputPacket.value + g_NetInputPacket.seed + g_NetInputPacket.unknown8;
            sender->Send(&g_NetInputPacket, sizeof(g_NetInputPacket));
            this->unknown460 = 1;
            this->AddRecord(g_Network->unknownAc, g_NetInputPacket.value, g_NetInputPacket.input, g_NetInputPacket.seed);
        }
        else
        {
            this->InsertRecord(network->unknownAc, this->unknown458, g_Rng.GetSeed());
            this->InsertRecord(1 - g_Network->unknownAc, this->unknown458, g_Rng.GetSeed());
            this->unknown458++;
        }
    }
    if (this->RecordsMeet(1 - g_Network->unknownAc))
    {
        this->unknown45c = this->unknown458;
        this->unknown460 = 0;
        inputs[0] = this->PopRecord(0, &seeds[0], &predicted);
        inputs[1] = this->PopRecord(1, &seeds[1], &predicted);
        UpdatePlayerInput(0, inputs[0]);
        UpdatePlayerInput(1, inputs[1]);
        UpdatePlayerInput(2, inputs[0] | inputs[1]);
        this->LeaveCriticalSection(4);
        return result;
    }
    now = timeGetTime();
    this->unknown46c = now;
    if (now < this->unknown464)
    {
        this->unknown464 = now;
    }
    if (now - this->unknown464 <= 20000)
    {
        goto waitForFrame;
    }
    if (now - this->unknown464 > 60000)
    {
        status = 3;
        goto end;
    }
    this->unknown478 = 1;
    g_SpriteEngine->RenderTextCentered(this->unknown584, 0xffffff, 0, "妖怪からの通信が途絶えています");
wait:
    status = 2;
end:
    this->LeaveCriticalSection(4);
    if (ReadNetInput() & 2)
    {
        return 3;
    }
    if ((this->stateFlags & 0x80) && !this->HasSubThread())
    {
        return 4;
    }
    return status;
}
