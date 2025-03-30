#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <cstdio>
#include <map>

namespace utils {
	class cmd_line_options {
	public:

		class string_option;
		class int_option;
		class bool_option;
		class yesno_option;
		class onoff_option;
		class action_option;

		class option {
		private:
			std::vector<std::string> val;
		public:
			enum val_type{
				VAL_NONE=0,
				VAL_STRING,
				VAL_BOOL,
				VAL_YES_NO,
				VAL_ON_OFF,
				VAL_INT
			};

			std::string short_arg;
			std::string long_arg;
			std::string help;
			bool no_args = false;
			bool required = false;
			std::string def_val;
			val_type type = VAL_NONE;

			option() :
				val(0),
				short_arg(""),
				long_arg(""),
				help(""),
				no_args(false),
				required(false),
				def_val(""),
				type(VAL_NONE) {
			}
			

			std::string short_name() const { return short_arg.empty() ? "" : "-" + short_arg; }
			std::string long_name() const { return long_arg.empty() ? "":"--" + long_arg; }
			
			const std::vector<std::string>& vals() const { return val; }

			std::string full_name() const {
				std::string result;
				if (is_null()) {
					return result;
				}
				auto s = short_name();
				auto l = long_name();
				if (!s.empty()) {
					result += s;
				}
				if (!l.empty()) {
					if (!result.empty()) {
						result += "/";
					}
					result += l;
				}
				return result; 
			}

			bool matches_name(const std::string& name) const {
				auto v = name;
				if (v[0] == '-') {
					v.erase(0, 1);
					if (v[0] == '-') {
						v.erase(0, 1);
					}
				}
				std::transform(v.begin(), v.end(), v.begin(), tolower);
				auto tmp1 = short_arg;
				std::transform(tmp1.begin(), tmp1.end(), tmp1.begin(), tolower);
				auto tmp2 = long_arg;
				std::transform(tmp2.begin(), tmp2.end(), tmp2.begin(), tolower);
				return v == tmp1 || v == tmp2;
			}

			bool empty() const {
				return short_arg.empty() && long_arg.empty() && help.empty() && def_val.empty();
			}

			option& operator+=(const std::string& v) {

				val.push_back(v);
				return *this;
			}

			option& operator=(const std::string& rhs) {
				val[0] = rhs;
				return *this;
			}

			option& operator=(bool rhs) {
				if (VAL_YES_NO == type) {
					val[0] = rhs ? "true" : "false";
				}
				if (VAL_ON_OFF == type) {
					val[0] = rhs ? "on" : "off";
				}
				else {
					val[0] = rhs ? "true" : "false";
				}
				return *this;
			}

			option& operator=(int rhs) {
				if (VAL_INT == type) {
					char tmp[256] = { 0 };
					snprintf(tmp, sizeof(tmp) - 1, "%d", rhs);
					val[0] = tmp;
				}
				else {
					val[0] = rhs ? "1" : "0";
				}
				return *this;
			}

			option operator[](int idx) const {
				if (is_null()) {
					return option::null();
				}
				if ((size_t)idx >= val.size()) {
					return option::null();
				}
				option result;
				result.type = type;
				if (val[idx].empty()) {
					result = def_val;
				}
				else {
					result = val[idx];
				}
				return result;
			}

			operator std::string() const {
				if (is_null()) {
					return "";
				}
				if (!val.empty()) {
					if (val[0].empty()) {
						return def_val;
					}
				}
				else {
					return def_val;
				}
				return val[0];
			}

			int to_int() const {
				int result = 0;
				if (is_null()) {
					return result;
				}
				if (!val.empty()) {
					if (val[0].empty()) {
						result = atoi(def_val.c_str());
					}
					else {
						result = atoi(val[0].c_str());
					}
				}
				else {
					result = atoi(def_val.c_str());
				}
				
				return result;
			}

			operator bool() const {
				return to_bool();
			}

			bool to_bool() const {
				bool result = false;

				if (is_null()) {
					return result;
				}

				auto tmp = def_val;
				if (!val.empty()) {
					if (!val[0].empty()) {
						tmp = val[0];
					}
				}
				
				std::transform(tmp.begin(), tmp.end(), tmp.begin(), std::tolower);
				if (type == VAL_YES_NO && (tmp == "yes" || tmp == "no")) {
					result = tmp == "yes" ? true : false;
				}
				else if (type == VAL_ON_OFF && (tmp == "on" || tmp == "off")) {
					result = tmp == "on" ? true : false;
				}
				else if (type == VAL_BOOL && (tmp == "true" || tmp == "false")) {
					result = tmp == "true" ? true : false;
				}
				else if (type == VAL_BOOL && (tmp == "1" || tmp == "0")) {
					result = tmp == "1" ? true : false;
				}
				return result;
			}

			bool is_null() const {
				return type == VAL_NONE && val.empty() && def_val.empty();
			}
			static const option& null() {
				static option nullOpt;
				return nullOpt;
			}
			
			friend class string_option;
			friend class int_option;
			friend class bool_option;
			friend class yesno_option;
			friend class onoff_option;
			friend class action_option;
		private:
			void init() {
				if (!def_val.empty() && type == VAL_STRING) {
					auto tmp = def_val;
					std::transform(tmp.begin(), tmp.end(), tmp.begin(), std::tolower);
					if (tmp == "yes" || tmp == "no") {
						type = VAL_YES_NO;
					}
					else if (tmp == "off" || tmp == "on") {
						type = VAL_ON_OFF;
					}

					if (val.empty()) {
						val.push_back(def_val);
					}
					else {
						val[0]=def_val;
					}
				}
			}
		};


		class string_option {
		public:
			option opt;
			string_option(const std::string& shortArg, const std::string& longArg, const std::string& hlp = "",
				bool req = false, const std::string& defV = "")  {

				//opt.val.resize(1);
				opt.short_arg = shortArg;
				opt.long_arg = longArg;
				opt.help = hlp;
				opt.no_args = false;
				opt.required = req;
				opt.def_val = defV;
				opt.type = option::VAL_STRING;


				opt.init();
			}
			operator option& () {
				return opt;
			}
		};

		class int_option {
		public:
			option opt;
			int_option(const std::string& shortArg, const std::string& longArg, int defIntVal, const std::string& hlp = "", bool required=false) {

				//opt.val.resize(1);
				opt.short_arg = shortArg;
				opt.long_arg = longArg;
				opt.help = hlp;
				opt.no_args = false;
				opt.required = required;
				opt.def_val = "";
				opt.type = option::VAL_INT;


				char tmp[256] = { 0 };
				snprintf(tmp, sizeof(tmp) - 1, "%d", defIntVal);
				opt.def_val = tmp;
				

				opt.init();
			}
			operator option& () {
				return opt;
			}
		};

		class bool_option {
		public:
			option opt;
			bool_option(const std::string& shortArg, const std::string& longArg, bool defBoolVal, const std::string& hlp = "", bool required = false) {

				//opt.val.resize(1);
				opt.short_arg = shortArg;
				opt.long_arg = longArg;
				opt.help = hlp;
				opt.no_args = false;
				opt.required = required;
				opt.def_val = defBoolVal ? "true" : "false";
				opt.type = option::VAL_BOOL;

				


				opt.init();
			}
			operator option& () {
				return opt;
			}
		};

		class yesno_option {
		public:
			option opt;
			yesno_option(const std::string& shortArg, const std::string& longArg, bool defBoolVal, const std::string& hlp = "", bool required = false) {

				//opt.val.resize(1);
				opt.short_arg = shortArg;
				opt.long_arg = longArg;
				opt.help = hlp;
				opt.no_args = false;
				opt.required = required;
				opt.def_val = defBoolVal ? "yes" : "no";
				opt.type = option::VAL_YES_NO;




				opt.init();
			}
			operator option& () {
				return opt;
			}
		};

		class onoff_option {
		public:
			option opt;
			onoff_option(const std::string& shortArg, const std::string& longArg, bool defBoolVal, const std::string& hlp = "", bool required = false) {

				//opt.val.resize(1);
				opt.short_arg = shortArg;
				opt.long_arg = longArg;
				opt.help = hlp;
				opt.no_args = false;
				opt.required = required;
				opt.def_val = defBoolVal ? "on" : "off";
				opt.type = option::VAL_ON_OFF;




				opt.init();
			}
			operator option& () {
				return opt;
			}
		};

		class action_option {
		public:
			option opt;
			action_option(const std::string& shortArg, const std::string& longArg, const std::string& hlp = "") {

				//opt.val.resize(1);
				opt.short_arg = shortArg;
				opt.long_arg = longArg;
				opt.help = hlp;
				opt.no_args = true;
				opt.required = false;
				opt.def_val = "true";
				opt.type = option::VAL_BOOL;
				//opt.val[0] = "true";

				opt.init();
			}
			operator option& () {
				return opt;
			}
		};

		class cmd_param {
		public:
			std::string name = "";
			std::vector<std::string> vals;
		};

		typedef std::map<std::string, cmd_param> cmd_line_map_t;
		typedef std::vector<option> option_array_t;

		

		class cmd_line {
		public:

			size_t param_count(const std::string& name) {
				size_t result = 0;
				if (has_switch(name)) {
					auto  it = get_switch(name);
					result = it->second.vals.size();
				}
				return result;
			}

			std::string param_val(const std::string& name, int idx) {
				std::string result = "";
				if (has_switch(name)) {
					auto  it = get_switch(name);
					if (idx < it->second.vals.size()) {
						result = it->second.vals[idx];
					}
					
				}
				return result;
			}

			const option& get_opt_for_name(const std::string& name, const option_array_t& opts) const {
				for (auto& o:opts) {
					if (o.matches_name(name)) {
						return o;
					}
				}
				return option::null();
			}

			operator std::string() const {
				std::string result;
				for (auto s: original_cmd_line) {
					result += s;
					result += " ";
				}

				return result;
			}


			void process(int argc, char** argv, const option_array_t& opts) {
				
				original_cmd_line.clear();

				for (int i = 0;i < argc;i++) {
					original_cmd_line.push_back(argv[i]);
				}
				std::string cur_param;

				for (size_t i = 0;i < original_cmd_line.size();i++) {
					auto val = original_cmd_line[i];
					if (is_switch(val)) {
						cur_param = val;
						std::string arg = "";
						bool clear_cur_param=false;
						auto& opt = get_opt_for_name(cur_param,opts);
						cmd_param param;
						auto found = cmd_line_data.find(cur_param);
						if (found != cmd_line_data.end()) {
							param = found->second;
						}
						else {
							param.name = cur_param;
						}
						
						if (!opt.no_args) {
							if (i + 1 < argc) {
								if (!is_switch(original_cmd_line[i + 1])) {
									arg = original_cmd_line[i + 1];
									i++;
								}
								else {
									arg = "";
								}
								
								clear_cur_param = true;
							}
							if (!arg.empty()) {
								param.vals.push_back(arg);
							}
						}
						
						if (found == cmd_line_data.end()) {
							cmd_line_data.insert(cmd_line_map_t::value_type(cur_param, param));
						}
						else {
							found->second = param;
						}

						if (opt.no_args) {							
							clear_cur_param = true;
						}

						if (clear_cur_param) {
							cur_param = "";
						}
					}
					else {
						auto cmd_it = cmd_line_data.find(cur_param);
						if (cmd_it!= cmd_line_data.end()) {
							cmd_it->second.vals.push_back(original_cmd_line[i]);
						}
						else {
							//cmd_param param;
							//cmd_line_data.insert(cmd_line_map_t::value_type(val, param));
							if (i > 0) {
								trailing_values.push_back(val);
							}
						}
					}

				}
			}

			cmd_line_map_t ::const_iterator get_switch(const std::string& v) const {
				if (v.empty()) {
					return cmd_line_data.end();
				}

				auto tmp = v;
				if (tmp[0] != '-') {
					tmp.insert(0, "-");
				}

				return cmd_line_data.find(tmp);
			}

			bool has_switch(const std::string& v) const {
				if (v.empty()) {
					return false;
				}
				auto tmp = v;
				if (tmp[0] != '-') {
					tmp.insert(0, "-");
				}

				return cmd_line_data.count(tmp) != 0;
			}

			const std::vector<std::string>& trailing() const { return trailing_values; }
		private:
			
			std::vector<std::string> original_cmd_line;
			cmd_line_map_t cmd_line_data;
			std::vector<std::string> trailing_values;

			bool is_switch(const std::string& v) {
				if (v.empty()) {
					return false;
				}
				if (v.size() <= 1) {
					return false;
				}
				if (v[0] == '-') {
					return (!isdigit(v[1]));
				}

				return false;
			}
		};

		cmd_line_options(const std::string& name, option opts[], int count) {
			set_opts(opts, count);
			set_app_name(name);
		}

		cmd_line_options( option opts[], int count) {
			set_opts(opts, count);
		}

		void set_opts(option opts[], int count) {
			for (int i = 0;i < count;++i) {
				if (!opts[i].is_null()) {
					options.push_back(opts[i]);
				}
			}
		}

		std::string cmdline_str() const {
			return command_line;
		}

		bool parse( int argc, char** argv ) {
			command_line.process(argc,argv, options);
			size_t parse_count = 0;
			option_array_t req_opts;
			required_options(req_opts);


			for (auto& opt:options) {
				
				if (has_opt(opt)) {
					auto arg = opt.short_name();
					if (!command_line.has_switch(arg)) {
						arg = opt.long_name();
					}

					auto param_count = command_line.param_count(arg);
					for (int i = 0;i < param_count;++i) {
						opt += command_line.param_val(arg,i);
					}
					parsed_options.push_back(opt);
					parse_count++;
				}
				
			}

			for (auto& opt : parsed_options) {
				if (opt.vals().empty()) {
					opt += opt.def_val;
				}
			}
			if (parse_count < req_opts.size()) {
				return false;
			}
			
			return true;
		}

		bool has_opt(const option& opt) {
			if (command_line.has_switch(opt.short_name()) || command_line.has_switch(opt.long_name())) {
				return true;
			}
			return false;
		}

		void set_app_name(const std::string& v) { app_name = v; }
		std::string get_app_name() const { return app_name; }

		const option& operator[](const std::string& name) const {
			for (auto& opt : parsed_options) {
				if (opt.matches_name(name)) {
					return opt;
				}
			}

			for (auto& opt : options) {
				if (opt.matches_name(name) && !opt.no_args && !opt.def_val.empty()) {
					return opt;
				}
			}
			return option::null();
		}


		void usage() const {
			std::cout << get_app_name() << " usage:" << std::endl;
			for (auto opt : options) {
				std::cout << opt.full_name();
				if (opt.required) {
					std::cout << " (required)";
				}
				else {
					std::cout << " (optional)";
				}

				if (opt.no_args) {
					std::cout << " (no params needed)";
				}
				else {
					switch (opt.type) {
						case option::VAL_STRING: { std::cout << " (string param)"; } break;
						case option::VAL_BOOL: { std::cout << " (bool param - true/false)"; } break;
						case option::VAL_YES_NO: { std::cout << " (bool param - yes/no)"; } break;
						case option::VAL_ON_OFF: { std::cout << " (bool param - on/off)"; } break;
						case option::VAL_INT: { std::cout << " (int param)"; } break;
					}
				}

				
				std::cout << " : " << opt.help << std::endl;
				if (!opt.def_val.empty() && !opt.no_args) {
					std::cout << "\tdefault: " << opt.def_val << std::endl;
				}
			}
		}


		option trailing_values() {
			option result;
			result.type = option::VAL_STRING;
			
			auto trailing = command_line.trailing();
			for (auto v : trailing) {
				
				result += v;
			}

			return result;
		}

		private:
			void required_options(option_array_t& opts) {
				opts.clear();
				for (auto  o: options) {
					if (o.required) {
						opts.push_back(o);
					}
				}
			}

			void optional_options(option_array_t& opts) {
				opts.clear();
				for (auto o : options) {
					if (!o.required) {
						opts.push_back(o);
					}
				}
			}

			cmd_line command_line;

			option_array_t options;
			option_array_t parsed_options;
			std::string app_name;
	};
}