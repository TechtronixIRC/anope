/* ChanServ core functions
 *
 * (C) 2003-2013 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 */

/*************************************************************************/

#include "module.h"

class CommandCSKick : public Command
{
 public:
	CommandCSKick(Module *creator) : Command(creator, "chanserv/kick", 2, 3)
	{
		this->SetDesc(_("Kicks a specified nick from a channel"));
		this->SetSyntax(_("\037channel\037 \037nick\037 [\037reason\037]"));
		this->SetSyntax(_("\037channel\037 \037mask\037 [\037reason\037]"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params) anope_override
	{
		const Anope::string &chan = params[0];
		const Anope::string &target = params[1];
		Anope::string reason = params.size() > 2 ? params[2] : "Requested";

		User *u = source.GetUser();
		ChannelInfo *ci = ChannelInfo::Find(params[0]);
		Channel *c = Channel::Find(params[0]);
		User *u2 = User::Find(target, true);

		if (!c)
		{
			source.Reply(CHAN_X_NOT_IN_USE, chan.c_str());
			return;
		}
		else if (!ci)
		{
			source.Reply(CHAN_X_NOT_REGISTERED, chan.c_str());
			return;
		}

		if (reason.length() > Config->CSReasonMax)
			reason = reason.substr(0, Config->CSReasonMax);

		AccessGroup u_access = source.AccessFor(ci);

		if (!u_access.HasPriv("KICK") && !source.HasPriv("chanserv/kick"))
			source.Reply(ACCESS_DENIED);
		else if (u2)
		{
			AccessGroup u2_access = ci->AccessFor(u2);
			if (u != u2 && ci->HasExt("PEACE") && u2_access >= u_access && !source.HasPriv("chanserv/kick"))
				source.Reply(ACCESS_DENIED);
			else if (u2->IsProtected())
				source.Reply(ACCESS_DENIED);
			else if (!c->FindUser(u2))
				source.Reply(NICK_X_NOT_ON_CHAN, u2->nick.c_str(), c->name.c_str());
			else
			{
				bool override = !u_access.HasPriv("KICK") || (u != u2 && ci->HasExt("PEACE") && u2_access >= u_access);
				Log(override ? LOG_OVERRIDE : LOG_COMMAND, source, this, ci) << "for " << u2->nick;

				if (ci->HasExt("SIGNKICK") || (ci->HasExt("SIGNKICK_LEVEL") && !u_access.HasPriv("SIGNKICK")))
					c->Kick(ci->WhoSends(), u2, "%s (%s)", reason.c_str(), source.GetNick().c_str());
				else
					c->Kick(ci->WhoSends(), u2, "%s", reason.c_str());
			}
		}
		else if (u_access.HasPriv("FOUNDER"))
		{
			Log(LOG_COMMAND, source, this, ci) << "for " << target;

			int matched = 0, kicked = 0;
			for (Channel::ChanUserList::iterator it = c->users.begin(), it_end = c->users.end(); it != it_end;)
			{
				ChanUserContainer *uc = *it++;

				if (Anope::Match(uc->user->nick, target) || Anope::Match(uc->user->GetDisplayedMask(), target))
				{
					++matched;

					AccessGroup u2_access = ci->AccessFor(uc->user);
					if (u != uc->user && ci->HasExt("PEACE") && u2_access >= u_access)
						continue;
					else if (uc->user->IsProtected())
						continue;

					++kicked;
					if (ci->HasExt("SIGNKICK") || (ci->HasExt("SIGNKICK_LEVEL") && !u_access.HasPriv("SIGNKICK")))
						c->Kick(ci->WhoSends(), uc->user, "%s (Matches %s) (%s)", reason.c_str(), target.c_str(), source.GetNick().c_str());
					else
						c->Kick(ci->WhoSends(), uc->user, "%s (Matches %s)", reason.c_str(), target.c_str());
				}
			}

			if (matched)
				source.Reply(_("Kicked %d/%d users matching %s from %s."), kicked, matched, target.c_str(), c->name.c_str());
			else
				source.Reply(_("No users on %s match %s."), c->name.c_str(), target.c_str());
		}
		else
			source.Reply(NICK_X_NOT_IN_USE, target.c_str());
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand) anope_override
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Kicks a specified nick from a channel.\n"
				" \n"
				"By default, limited to AOPs or those with level 5 access\n"
				"and above on the channel. Channel founders may use masks too."));
		return true;
	}
};

class CSKick : public Module
{
	CommandCSKick commandcskick;

 public:
	CSKick(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, CORE), commandcskick(this)
	{
		this->SetAuthor("Anope");

	}
};

MODULE_INIT(CSKick)
