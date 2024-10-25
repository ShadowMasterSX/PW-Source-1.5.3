#ifndef __ONLINE_GAME_GS_PLAYER_SANCTUARY_CHECK_H__
#define __ONLINE_GAME_GS_PLAYER_SANCTUARY_CHECK_H__


class base_wrapper;
class gplayer_imp;

class player_sanctuary_check
{
public:
    player_sanctuary_check();
    void SetHasPVPLimitFilter(bool has_pvp_limit_filter);
    void OnStartEnterSanctuarySession();
    void OnHeartbeat(gplayer_imp* pImpl);

    bool Save(archive& ar);
    bool Load(archive& ar);
    void Swap(player_sanctuary_check& rhs);

private:
    bool IsInSanctuary(gplayer_imp* pImpl);
    void OnNormalHeartbeat(gplayer_imp* pImpl);
    void OnCheatHeartbeat(gplayer_imp* pImpl);
    
    int _wait_times;
    int _cheat_times;
    int _counter;
    int _status;
    bool _has_pvp_limit_filter;
};

#endif

