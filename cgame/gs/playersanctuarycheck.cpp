#include "world.h"
#include "player_imp.h"
#include "common/packetwrapper.h"
#include "playertemplate.h"
#include "playersanctuarycheck.h"
#include <glog.h>

#define SANCTUARY_CHECK_TIME_NORMAL 8
#define SANCTUARY_CHECK_TIME_CHEAT (SANCTUARY_CHECK_TIME_NORMAL / 2)
#define SANCTUARY_MAX_WAIT_TIMES 2
#define SANCTUARY_MAX_CHEAT_TIMES 2

enum
{
    SANCTUARY_CHECK_STATUS_NORMAL,
    SANCTUARY_CHECK_STATUS_CHEAT,
};


player_sanctuary_check::player_sanctuary_check()
{
    _wait_times = 0;
    _cheat_times = 0;
    _counter = 0;
    _status = SANCTUARY_CHECK_STATUS_NORMAL;
    _has_pvp_limit_filter = false;
}

void player_sanctuary_check::SetHasPVPLimitFilter(bool has_pvp_limit_filter)
{
    _has_pvp_limit_filter = has_pvp_limit_filter;
}

void player_sanctuary_check::OnStartEnterSanctuarySession()
{
    if (_status == SANCTUARY_CHECK_STATUS_CHEAT) return;

    _wait_times = 0;
    if (_cheat_times > 0) --_cheat_times;
}

bool player_sanctuary_check::IsInSanctuary(gplayer_imp* pImpl)
{
    if (!pImpl->_parent->pPiece->region_overlap) return false;
    return player_template::IsInSanctuary(pImpl->_parent->pos);
}

void player_sanctuary_check::OnNormalHeartbeat(gplayer_imp* pImpl)
{
    if (++_counter < SANCTUARY_CHECK_TIME_NORMAL) return;

    _counter = 0;
    if (_has_pvp_limit_filter) return;

    if (IsInSanctuary(pImpl))
    {
        if (++_wait_times >= SANCTUARY_MAX_WAIT_TIMES)
        {
            _wait_times = 0;
            pImpl->PlayerAddPVPLimitFilter();

            if (++_cheat_times >= SANCTUARY_MAX_CHEAT_TIMES)
            {
                _status = SANCTUARY_CHECK_STATUS_CHEAT;
                GLog::log(GLOG_ERR, "用户%d安全区状态可疑", pImpl->_parent->ID.id);
            }
        }
    }
    else
    {
        _wait_times = 0;
    }
}

void player_sanctuary_check::OnCheatHeartbeat(gplayer_imp* pImpl)
{
    if (++_counter < SANCTUARY_CHECK_TIME_CHEAT) return;

    _counter = 0;
    if (_has_pvp_limit_filter) return;

    if (IsInSanctuary(pImpl))
        pImpl->PlayerAddPVPLimitFilter();
}

void player_sanctuary_check::OnHeartbeat(gplayer_imp* pImpl)
{
    if (_status == SANCTUARY_CHECK_STATUS_NORMAL)
        OnNormalHeartbeat(pImpl);
    else if (_status == SANCTUARY_CHECK_STATUS_CHEAT)
        OnCheatHeartbeat(pImpl);
}

bool player_sanctuary_check::Save(archive& ar)
{
    ar << _wait_times;
    ar << _cheat_times;
    ar << _counter;
    ar << _status;
    ar << _has_pvp_limit_filter;

    return true;
}

bool player_sanctuary_check::Load(archive& ar)
{
    ar >> _wait_times;
    ar >> _cheat_times;
    ar >> _counter;
    ar >> _status;
    ar >> _has_pvp_limit_filter;

    return true;
}

void player_sanctuary_check::Swap(player_sanctuary_check& rhs)
{
    abase::swap(_wait_times, rhs._wait_times);
    abase::swap(_cheat_times, rhs._cheat_times);
    abase::swap(_counter, rhs._counter);
    abase::swap(_status, rhs._status);
    abase::swap(_has_pvp_limit_filter, rhs._has_pvp_limit_filter);
}


