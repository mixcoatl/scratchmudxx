//! \file game_action.cpp
//!
//! \par Copyright
//! Copyright (C) 1999-2026 scratchmud.org
//! All rights reserved.
//!
//! \author Geoffrey Davis (gdavis@scratchmud.org)

#define _SCRATCH_GAME_ACTION_CPP_

#include <scratch/action.hpp>
#include <scratch/color.hpp>
#include <scratch/color_bindings.hpp>
#include <scratch/command.hpp>
#include <scratch/command_bindings.hpp>
#include <scratch/descriptor.hpp>
#include <scratch/game.hpp>
#include <scratch/gender.hpp>
#include <scratch/lua.hpp>
#include <scratch/parser.hpp>
#include <scratch/scratch.hpp>
#include <scratch/storage_file_multi.hpp>
#include <scratch/string.hpp>
#include <scratch/user.hpp>
#include <scratch/user_bindings.hpp>

#include <cctype>

namespace Scratch {
namespace Core {

using Lua = Scratch::Scripting::Lua;

namespace {

//! Returns whether the key starts with the prefix.
//! \param key the candidate string
//! \param prefix the prefix
bool StartsWithCi(
	const String& key,
	const String& prefix) noexcept {
    if (prefix.size() > key.size())
	return false;
    return Scratch::Algorithm::Strings::CompareCi(
	    key.substr(0, prefix.size()), prefix) == 0;
}

//! Returns the genitive form.
//! \param name the name
String MakeNamePossessive(const String& name) {
    if (name.empty())
	return "<Invalid>";
    const auto lower = [](const char c) {
	return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    };
    const char last = lower(name.back());
    if (last == 's' || last == 'x' || last == 'z')
	return name + "'";
    if (name.size() >= 2) {
	const char a = lower(name[name.size() - 2]);
	const char b = lower(name[name.size() - 1]);
	if ((a == 'c' && b == 'h') || (a == 's' && b == 'h'))
	    return name + "'";
    }
    return name + "'s";
}

//! Returns whether the text starts with a vowel.
//! \param text the text
bool StartsWithVowel(const String& text) noexcept {
    if (text.empty())
	return false;
    const char c = static_cast<char>(
	std::tolower(static_cast<unsigned char>(text[0])));
    return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
}

//! Resolves gender.
//! \param thing the thing
//! \return the gender, or \c GENDER_UNDEFINED
Gender::GenderEnum ResolveGender(const ThingPtr& thing) {
    auto user = std::dynamic_pointer_cast<User>(thing);
    if (!user)
	return Gender::GENDER_UNDEFINED;
    return user->GetGender();
}

//! Resolves one property.
//! \param prop the property name
//! \param param the action param
//! \param recipient the recipient thing
//! \param raw whether you-shift is disabled
String ResolveProperty(
	const String& prop,
	const ActionParam& param,
	const ThingPtr& recipient,
	const bool raw) {
    const auto thing = param.GetThing();
    const bool isMe = !raw && thing && recipient && thing == recipient;
    const auto gender = ResolveGender(thing);

    if (Scratch::Algorithm::Strings::CompareCi(prop, "Text") == 0) {
	if (!param.GetText().empty())
	    return param.GetText();
	if (thing)
	    return thing->GetName();
	return String();
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "An") == 0) {
	String text = param.GetText();
	if (text.empty() && thing)
	    text = thing->GetName();
	return StartsWithVowel(text) ? "an" : "a";
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "Name") == 0 ||
	    Scratch::Algorithm::Strings::CompareCi(prop, "RawName") == 0) {
	if (!thing)
	    return "<Invalid>";
	if (isMe)
	    return "you";
	return thing->GetName();
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "NamePossessive") == 0 ||
	    Scratch::Algorithm::Strings::CompareCi(prop, "RawNamePossessive") == 0) {
	if (!thing)
	    return "<Invalid>";
	if (isMe)
	    return "your";
	return MakeNamePossessive(thing->GetName());
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "Copula") == 0 ||
	    Scratch::Algorithm::Strings::CompareCi(prop, "RawCopula") == 0) {
	if (!thing)
	    return "<Invalid>";
	if (isMe)
	    return "are";
	return Gender::GetCopula(gender);
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "Determiner") == 0 ||
	    Scratch::Algorithm::Strings::CompareCi(prop, "RawDeterminer") == 0) {
	if (!thing)
	    return "<Invalid>";
	if (isMe)
	    return "your";
	return Gender::GetDeterminer(gender);
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "Subject") == 0 ||
	    Scratch::Algorithm::Strings::CompareCi(prop, "RawSubject") == 0) {
	if (!thing)
	    return "<Invalid>";
	if (isMe)
	    return "you";
	return Gender::GetSubject(gender);
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "Object") == 0 ||
	    Scratch::Algorithm::Strings::CompareCi(prop, "RawObject") == 0) {
	if (!thing)
	    return "<Invalid>";
	if (isMe)
	    return "you";
	return Gender::GetObject(gender);
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "Possessive") == 0 ||
	    Scratch::Algorithm::Strings::CompareCi(prop, "RawPossessive") == 0) {
	if (!thing)
	    return "<Invalid>";
	if (isMe)
	    return "yours";
	return Gender::GetPossessive(gender);
    }
    if (Scratch::Algorithm::Strings::CompareCi(prop, "Reflexive") == 0 ||
	    Scratch::Algorithm::Strings::CompareCi(prop, "RawReflexive") == 0) {
	if (!thing)
	    return "<Invalid>";
	if (isMe)
	    return "yourself";
	return Gender::GetReflexive(gender);
    }
    return "<Invalid>";
}

//! Resolves a slot letter to a param.
//! \param slot the slot letter
//! \param subject the subject slot
//! \param direct the direct slot
//! \param indirect the indirect slot
//! \param extra the extra slot
//! \return the matched param, or \c nullptr
const ActionParam* SlotParam(
	const char slot,
	const ActionParam& subject,
	const ActionParam& direct,
	const ActionParam& indirect,
	const ActionParam& extra) noexcept {
    switch (static_cast<char>(
	    std::toupper(static_cast<unsigned char>(slot)))) {
    case 'S': return &subject;
    case 'D': return &direct;
    case 'I': return &indirect;
    case 'X': return &extra;
    default: return nullptr;
    }
}

//! Expands one macro side.
//! \param side the macro side
//! \param subject the subject slot
//! \param direct the direct slot
//! \param indirect the indirect slot
//! \param extra the extra slot
//! \param recipient the recipient thing
String ExpandSide(
	const String& side,
	const ActionParam& subject,
	const ActionParam& direct,
	const ActionParam& indirect,
	const ActionParam& extra,
	const ThingPtr& recipient) {
    const auto dot = side.find('.');
    if (dot == String::npos)
	return side;
    if (dot == 0 || side.size() < 3)
	return "<Invalid>";
    const auto* param = SlotParam(
	    side[0], subject, direct, indirect, extra);
    if (!param)
	return "<Invalid>";
    const auto prop = side.substr(dot + 1);
    const bool raw = Scratch::Algorithm::Strings::CompareCi(
	    prop.substr(0, 3), "Raw") == 0;
    return ResolveProperty(prop, *param, recipient, raw);
}

//! Expands an action message template.
//! \param message the message template
//! \param subject the subject slot
//! \param direct the direct slot
//! \param indirect the indirect slot
//! \param extra the extra slot
//! \param recipient the recipient thing
String ExpandMessage(
	const String& message,
	const ActionParam& subject,
	const ActionParam& direct,
	const ActionParam& indirect,
	const ActionParam& extra,
	const ThingPtr& recipient) {
    String out;
    out.reserve(message.size());
    for (std::size_t i = 0; i < message.size(); ++i) {
	if (message[i] != '$') {
	    out.push_back(message[i]);
	    continue;
	}
	if (i + 1 >= message.size()) {
	    out.push_back('$');
	    continue;
	}
	if (message[i + 1] == '$') {
	    out.push_back('$');
	    ++i;
	    continue;
	}
	if (message[i + 1] != '{') {
	    out.push_back('$');
	    continue;
	}
	const std::size_t begin = i + 2;
	std::size_t end = begin;
	while (end < message.size() && message[end] != '}')
	    ++end;
	if (end >= message.size()) {
	    out.append(message.substr(i));
	    break;
	}
	const auto body = message.substr(begin, end - begin);
	const auto colon = body.find(':');
	String chosen;
	if (colon == String::npos) {
	    chosen = body;
	} else {
	    const bool youSubject =
		    subject.GetThing() && recipient &&
		    subject.GetThing() == recipient;
	    chosen = youSubject ?
		    body.substr(0, colon) :
		    body.substr(colon + 1);
	}
	out += ExpandSide(
		chosen, subject, direct, indirect, extra, recipient);
	i = end;
    }
    return out;
}

} // namespace

//! Sends an action message.
//! \param metacolor the message metacolor
//! \param targets the audience bits
//! \param message the message template
//! \param subject the subject slot
//! \param direct the direct slot
//! \param indirect the indirect slot
//! \param extra the extra slot
void Game::Action(
	const Color::ColorEnum metacolor,
	const unsigned targets,
	const String& message,
	const ActionParam& subject,
	const ActionParam& direct,
	const ActionParam& indirect,
	const ActionParam& extra) {
    if (message.empty() || targets == 0)
	return;

    std::set<ThingPtr> audience;
    auto addThing = [&audience](const ActionParam& param) {
	if (param.GetThing())
	    audience.insert(param.GetThing());
    };
    addThing(subject);
    addThing(direct);
    addThing(indirect);
    addThing(extra);

    for (auto& d: this->GetDescriptors()) {
	if (!d || d->Closed())
	    continue;
	auto user = d->GetUser();
	if (user)
	    audience.insert(user);
    }

    const auto subjectThing = subject.GetThing();
    const auto victThing = direct.GetThing() ?
	    direct.GetThing() :
	    (indirect.GetThing() ? indirect.GetThing() : ThingPtr());

    for (const auto& recipient: audience) {
	if (!recipient)
	    continue;
	const bool isChar =
		subjectThing && recipient == subjectThing;
	const bool isVict =
		victThing && recipient == victThing;
	const bool isOther = !isChar && !isVict;

	if (isChar && !(targets & ACT_TOCHAR))
	    continue;
	if (isVict && !(targets & ACT_TOVICT))
	    continue;
	if (isOther && !(targets & ACT_TONOTVICT))
	    continue;

	auto d = this->GetDescriptorFor(recipient);
	if (!d || d->Closed())
	    continue;

	if (isChar && (targets & ACT_NOREPEAT)) {
	    auto user = std::dynamic_pointer_cast<User>(recipient);
	    if (user && user->HasPreference("NoRepeat")) {
		String out;
		out += d->GetColor(Color::C_OKAY);
		out += "OK.";
		out += d->GetColor(Color::C_NORMAL);
		out += "\r\n";
		d->Print(out);
		continue;
	    }
	}

	this->ActionPerform(
		metacolor, message, subject, direct, indirect, extra,
		recipient, *d);
    }
}

//! Expands and prints one action message.
//! \param metacolor the message metacolor
//! \param message the message template
//! \param subject the subject slot
//! \param direct the direct slot
//! \param indirect the indirect slot
//! \param extra the extra slot
//! \param recipient the recipient thing
//! \param to the recipient descriptor
//! \sa #Action
void Game::ActionPerform(
	const Color::ColorEnum metacolor,
	const String& message,
	const ActionParam& subject,
	const ActionParam& direct,
	const ActionParam& indirect,
	const ActionParam& extra,
	const ThingPtr& recipient,
	Descriptor& to) {
    auto expanded = ExpandMessage(
	    message, subject, direct, indirect, extra, recipient);
    if (expanded.empty())
	return;
    Scratch::Algorithm::Strings::Capitalize(expanded);

    String out;
    out += to.GetColor(metacolor);
    out += expanded;
    out += to.GetColor(Color::C_NORMAL);
    out += "\r\n";
    to.Print(out);
}

//! Finds a command.
//! \param word the first input word
//! \param performer the performing thing
//! \return the matched command, or \c nullptr
//! \sa Command::Allows(const UserPtr&) const
//! \sa #GetCommandsIndex() const
CommandPtr Game::FindCommand(
	const String& word,
	const ThingPtr& performer) const noexcept {
    if (word.empty())
	return nullptr;

    const auto user = std::dynamic_pointer_cast<User>(performer);
    CommandPtr best;
    std::size_t bestLen = static_cast<std::size_t>(-1);
    for (auto it = commandsIndex_.lower_bound(word);
	    it != std::end(commandsIndex_); ++it) {
	if (!StartsWithCi(it->first, word))
	    break;
	if (!it->second || !it->second->Allows(user))
	    continue;
	if (it->first.size() < bestLen) {
	    best = it->second;
	    bestLen = it->first.size();
	}
    }
    return best;
}

//! Dispatches a command line.
//! \param performer the performing thing
//! \param line the raw input line
void Game::DispatchCommand(
	const ThingPtr& performer,
	const String& line) {
    if (!performer)
	return;

    String argument;
    const auto word = Scratch::Algorithm::Strings::ChopCopy(line, argument);
    if (word.empty())
	return;

    auto command = this->FindCommand(word, performer);
    if (!command) {
	auto user = std::dynamic_pointer_cast<User>(performer);
	if (user && user->HasPreference("AutoSay")) {
	    command = this->GetCommands()->Get("Say");
	    argument = line;
	}
    }

    if (command) {
	this->RunCommandHook(command, performer, argument);
	return;
    }

    auto d = this->GetDescriptorFor(performer);
    if (d && !d->Closed()) {
	String out;
	out += d->GetColor(Color::C_FAILED);
	out += "Huh!?";
	out += d->GetColor(Color::C_NORMAL);
	out += "\r\n";
	d->Print(out);
    }
}

//! Runs a command Action Lua hook with \c actor, \c command, \c line, and \c Q.
//! \param command the command
//! \param performer the performing thing
//! \param line the remainder after the matched word
void Game::RunCommandHook(
	const CommandPtr& command,
	const ThingPtr& performer,
	const String& line) {
    if (!command || !performer)
	return;
    const auto action = command->GetAction();
    const auto social = command->GetSocial();
    if (action.empty() && social) {
	if (auto user = std::dynamic_pointer_cast<User>(performer))
	    this->RunSocial(user, social, line);
	return;
    }
    if (action.empty())
	return;

    auto& lua = this->GetLua();
    Lua::Caller caller(lua, command->GetName() + ":Action");
    if (!caller.IsActive())
	return;

    if (auto user = std::dynamic_pointer_cast<User>(performer))
	Scripting::UserBindings::Push(lua, std::move(user));
    else
	lua.PushString(String());
    lua.SetEnv("actor");

    Scripting::CommandBindings::Push(lua, command);
    lua.SetEnv("command");

    lua.PushString(line);
    lua.SetEnv("line");

    if (auto d = this->GetDescriptorFor(performer))
	Scripting::ColorBindings::AssignQ(lua, *d);

    lua.Execute(action);
}

//! Runs social templates for \p actor.
//! \param actor the performing user
//! \param social the social templates
//! \param line the remainder after the matched command word
//! \sa #RunCommandHook(const CommandPtr&, const ThingPtr&, const String&)
void Game::RunSocial(
	const UserPtr& actor,
	const SocialPtr& social,
	const String& line) {
    if (!actor || !social)
	return;

    Parser parser;
    if (!parser.Parse(line)) {
	auto d = this->GetDescriptorFor(actor);
	if (d && !d->Closed()) {
	    String out;
	    out += d->GetColor(Color::C_FAILED);
	    out += "You don't see them here.";
	    out += d->GetColor(Color::C_NORMAL);
	    out += "\r\n";
	    d->Print(out);
	}
	return;
    }

    String message;
    ActionParam direct;
    if (!parser.GetSize()) {
	message = social->GetNoArgument();
    } else if (parser.GetSize() != 1 ||
	    parser.GetPhrase(0).GetWords().size() != 1) {
	auto d = this->GetDescriptorFor(actor);
	if (d && !d->Closed()) {
	    String out;
	    out += d->GetColor(Color::C_FAILED);
	    out += "You don't see them here.";
	    out += d->GetColor(Color::C_NORMAL);
	    out += "\r\n";
	    d->Print(out);
	}
	return;
    } else {
	auto target = actor->Find(
		*this, parser.GetPhrase(0).GetWords().front());
	if (!target) {
	    auto d = this->GetDescriptorFor(actor);
	    if (d && !d->Closed()) {
		String out;
		out += d->GetColor(Color::C_FAILED);
		out += "You don't see them here.";
		out += d->GetColor(Color::C_NORMAL);
		out += "\r\n";
		d->Print(out);
	    }
	    return;
	}
	ThingPtr actorThing = actor;
	if (target == actorThing) {
	    message = social->GetFoundAuto();
	    if (message.empty())
		message = social->GetFound();
	} else {
	    message = social->GetFound();
	}
	direct = ActionParam(target);
    }
    if (message.empty())
	return;

    this->Action(
	    Color::C_SOCIAL,
	    ACT_TOALL | ACT_NOREPEAT,
	    message,
	    ActionParam(actor),
	    direct);
}

//! Gets the controlling descriptor.
//! \param thing the thing
//! \return the controlling descriptor, or \c nullptr
DescriptorPtr Game::GetDescriptorFor(const ThingPtr& thing) noexcept {
    if (!thing)
	return nullptr;
    for (auto& d: this->GetDescriptors()) {
	if (!d || d->Closed())
	    continue;
	auto user = d->GetUser();
	if (user && user == thing)
	    return d;
    }
    return nullptr;
}

//! Rebuilds the keyword command index.
//! \throw std::runtime_error on keyword conflicts
//! \sa #GetCommandsIndex() const
void Game::RebuildCommandIndex() {
    commandsIndex_.clear();
    if (!commands_)
	return;

    for (const auto& id: commands_->GetIds()) {
	auto command = commands_->Get(id);
	if (!command)
	    continue;
	const auto name = command->GetName();
	if (name.empty())
	    continue;
	auto& slot = commandsIndex_[name];
	if (slot && slot != command) {
	    throw std::runtime_error(
		    "Command index conflict on name: " + name);
	}
	slot = command;
    }

    for (const auto& id: commands_->GetIds()) {
	auto command = commands_->Get(id);
	if (!command)
	    continue;
	for (const auto& keyword: command->GetKeywords()) {
	    if (keyword.empty())
		continue;
	    auto& slot = commandsIndex_[keyword];
	    if (slot && slot != command) {
		throw std::runtime_error(
			"Command index conflict on keyword: " + keyword);
	    }
	    slot = command;
	}
    }
}

}; // namespace Core
}; // namespace Scratch
