#ifdef WIN32
#include <winsock2.h>
#include "gncompress.h"
#else
#include "binder.h"
#endif
#include "gquerypasswd.hrp"
#include "userlogin.hrp"
#include "userlogin2.hrp"
#include "userlogout.hrp"
#include "cashserial.hrp"
#include "getaddcashsn.hrp"
#include "matrixpasswd.hrp"
#include "matrixpasswd2.hrp"
#include "matrixtoken.hrp"
#include "getusercoupon.hrp"
#include "couponexchange.hrp"
#include "queryuserprivilege_re.hpp"
#include "queryuserforbid_re.hpp"
#include "kickoutuser.hpp"
#include "accountingresponse.hpp"
#include "authdversion.hpp"
#include "au2game.hpp"
#include "discountannounce.hpp"
#include "keyexchange.hpp"
#include "statusannounce.hpp"
#include "accountingrequest.hpp"
#include "announcezoneid.hpp"
#include "queryuserprivilege.hpp"
#include "queryuserforbid.hpp"
#include "gmkickoutuser.hpp"
#include "gmforbidsellpoint.hpp"
#include "gmshutup.hpp"
#include "transbuypoint.hpp"
#include "getplayeridbyname_re.hpp"
#include "syssendmail_re.hpp"
#include "syssendmail3_re.hpp"
#include "verifymaster.hpp"
#include "verifymaster_re.hpp"
#include "certrequest.hpp"
#include "certkey.hpp"
#include "addcash.hpp"
#include "addcash_re.hpp"
#include "matrixfailure.hpp"
#include "billingrequest.hpp"
#include "billingbalance.hpp"
#include "billingbalancesa.hpp"
#include "billingconfirm.hpp"
#include "billingcancel.hpp"
#include "acforbidcheater.hpp"
#include "game2au.hpp"
#include "ssogetticketreq.hpp"

namespace GNET
{

static GQueryPasswd __stub_GQueryPasswd (RPC_GQUERYPASSWD, new GQueryPasswdArg, new GQueryPasswdRes);
static UserLogin __stub_UserLogin (RPC_USERLOGIN, new UserLoginArg, new UserLoginRes);
static UserLogin2 __stub_UserLogin2 (RPC_USERLOGIN2, new UserLogin2Arg, new UserLogin2Res);
static UserLogout __stub_UserLogout (RPC_USERLOGOUT, new UserLogoutArg, new UserLogoutRes);
static CashSerial __stub_CashSerial (RPC_CASHSERIAL, new CashSerialArg, new CashSerialRes);
static GetAddCashSN __stub_GetAddCashSN (RPC_GETADDCASHSN, new GetAddCashSNArg, new GetAddCashSNRes);
static MatrixPasswd __stub_MatrixPasswd (RPC_MATRIXPASSWD, new MatrixPasswdArg, new MatrixPasswdRes);
static MatrixPasswd2 __stub_MatrixPasswd2 (RPC_MATRIXPASSWD2, new MatrixPasswdArg, new MatrixPasswd2Res);
static MatrixToken __stub_MatrixToken (RPC_MATRIXTOKEN, new MatrixTokenArg, new MatrixTokenRes);
static GetUserCoupon __stub_GetUserCoupon (RPC_GETUSERCOUPON, new GetUserCouponArg, new GetUserCouponRes);
static CouponExchange __stub_CouponExchange (RPC_COUPONEXCHANGE, new CouponExchangeArg, new CouponExchangeRes);
static QueryUserPrivilege_Re __stub_QueryUserPrivilege_Re((void*)0);
static QueryUserForbid_Re __stub_QueryUserForbid_Re((void*)0);
static KickoutUser __stub_KickoutUser((void*)0);
static AccountingResponse __stub_AccountingResponse((void*)0);
static AuthdVersion __stub_AuthdVersion((void*)0);
static AU2Game __stub_AU2Game((void*)0);
static DiscountAnnounce __stub_DiscountAnnounce((void*)0);
static KeyExchange __stub_KeyExchange((void*)0);
static StatusAnnounce __stub_StatusAnnounce((void*)0);
static AccountingRequest __stub_AccountingRequest((void*)0);
static AnnounceZoneid __stub_AnnounceZoneid((void*)0);
static QueryUserPrivilege __stub_QueryUserPrivilege((void*)0);
static QueryUserForbid __stub_QueryUserForbid((void*)0);
static GMKickoutUser __stub_GMKickoutUser((void*)0);
static GMForbidSellPoint __stub_GMForbidSellPoint((void*)0);
static GMShutup __stub_GMShutup((void*)0);
static TransBuyPoint __stub_TransBuyPoint((void*)0);
static GetPlayerIDByName_Re __stub_GetPlayerIDByName_Re((void*)0);
static SysSendMail_Re __stub_SysSendMail_Re((void*)0);
static SysSendMail3_Re __stub_SysSendMail3_Re((void*)0);
static VerifyMaster __stub_VerifyMaster((void*)0);
static VerifyMaster_Re __stub_VerifyMaster_Re((void*)0);
static CertRequest __stub_CertRequest((void*)0);
static CertKey __stub_CertKey((void*)0);
static AddCash __stub_AddCash((void*)0);
static AddCash_Re __stub_AddCash_Re((void*)0);
static MatrixFailure __stub_MatrixFailure((void*)0);
static BillingRequest __stub_BillingRequest((void*)0);
static BillingBalance __stub_BillingBalance((void*)0);
static BillingBalanceSA __stub_BillingBalanceSA((void*)0);
static BillingConfirm __stub_BillingConfirm((void*)0);
static BillingCancel __stub_BillingCancel((void*)0);
static ACForbidCheater __stub_ACForbidCheater((void*)0);
static Game2AU __stub_Game2AU((void*)0);
static SSOGetTicketReq __stub_SSOGetTicketReq((void*)0);

};
