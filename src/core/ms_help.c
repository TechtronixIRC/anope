/* MemoServ core functions
 *
 * (C) 2003-2009 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 * $Id$
 *
 */
/*************************************************************************/

#include "module.h"

class CommandMSHelp : public Command
{
 public:
	CommandMSHelp() : Command("HELP", 1, 1)
	{
		this->SetFlag(CFLAG_ALLOW_UNREGISTERED);
	}

	CommandReturn Execute(User *u, const std::vector<ci::string> &params)
	{
		mod_help_cmd(Config.s_MemoServ, u, MEMOSERV, params[0].c_str());
		return MOD_CONT;
	}

	void OnSyntaxError(User *u, const ci::string &subcommand)
	{
		notice_help(Config.s_MemoServ, u, MEMO_HELP_HEADER);
		FOREACH_MOD(I_OnMemoServHelp, OnMemoServHelp(u));
		notice_help(Config.s_MemoServ, u, MEMO_HELP_FOOTER, Config.s_ChanServ);
	}
};

class MSHelp : public Module
{
 public:
	MSHelp(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetVersion("$Id$");
		this->SetType(CORE);
		this->AddCommand(MEMOSERV, new CommandMSHelp());
	}
};

MODULE_INIT(MSHelp)
