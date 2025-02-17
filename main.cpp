#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"

#include <GLFW/glfw3.h>

#include "imgui_stdlib.cpp" // std::string ImGui::InputText() support

#include "imgui_internal.h" // for void ToggleButton() function

#include "incs.h"

static void HelpMarker(const char* marker, const char* desc)
{
	ImGui::TextDisabled(marker);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

inline void ToggleButton(const char* str_id, bool* v)
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight();
	float width = height * 2.55f;
	float radius = height * 0.35f;
	float rounding = 0.2f;

	ImGui::InvisibleButton(str_id, ImVec2(width, height));
	if (ImGui::IsItemClicked()) *v = !*v;
	ImGuiContext& gg = *GImGui;
	float ANIM_SPEED = 0.085f;
	if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
		float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
	if (ImGui::IsItemHovered())
		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height),
			ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.68f, 0.68f, 0.68f, 1.0f)), height * rounding);
	else
		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height),
			ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.45f, 0.45f, 0.45f, 1.0f)), height * rounding);

	ImVec2 center = ImVec2(radius + (*v ? 1 : 0) * (width - radius * 4.0f), radius);
	draw_list->AddRectFilled(ImVec2((p.x + center.x) - 7.0f, p.y + 1.5f),
		ImVec2((p.x + (width / 2) + center.x) - 9.0f, p.y + height - 1.5f), IM_COL32(255, 255, 255, 255), height * rounding);
}

template <typename T, typename U>
inline std::pair<T, U> operator+=(std::pair<T, U>& l, const std::pair<T, U>& r) { return { l.first += r.first, l.second += r.second }; }

inline const int FONT_SCALE = 1000;

inline const float NEWLINE_MARGIN = 29;

static bool SHOW_DEMO = true;
static bool SHOW_FILE = false;
static bool SHOW_CHRONO = false;
static bool CASE_SENSETIVE = true;

inline const char *ALGORITHMS[] = { "Naive", "Knuth-Morris-Pratt", "Colussi", "Horspool*", "Rabin-Karp" };

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		// Get screen resolution
		static const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		// Scale font size of resolution height
		ImGui::SetWindowFontScale(static_cast<float>(mode->height) / FONT_SCALE);



		// std::string's that will store realtime inputted text and sample
		static std::string text, sample, text_upper, sample_upper;

		// Save length of text and sample
		unsigned int text_length = 0;
		unsigned int sample_length = 0;
		
		if (SHOW_DEMO) {
			ImGui::Begin("IO Demo", &SHOW_DEMO);

			// Scale font size of resolution height
			ImGui::SetWindowFontScale(static_cast<float>(mode->height) / FONT_SCALE);



			// Get text and sample input
			

			ImGui::PushItemWidth(ImGui::GetWindowWidth() - 17);

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::InputTextWithHint("  ", " Input text here", &text);

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::InputTextWithHint(" ", " Input string here", &sample);

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::PopItemWidth();

			text_length = text.length();
			sample_length = sample.length();




			// Vector that will store indices of sample beginings
			std::vector<unsigned int> indices;



			// Simple brute force algoritm to find sample in text
			//const double naive_time = naive_search(text, sample, indices);

			// KMP algorithm
			if (CASE_SENSETIVE)
				kmp_search(text, sample, indices);
			else {
				text_upper = boost::to_upper_copy<std::string>(text);
				sample_upper = boost::to_upper_copy<std::string>(sample);
				kmp_search(text_upper, sample_upper, indices);
			}

			// Save amount of indices
			const unsigned int amount = indices.size();



			static bool show_output = true;
			ImGui::Checkbox("Output:", &show_output);
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (show_output) {
				std::string fragment;

				float margin = NEWLINE_MARGIN, line_length = 0;

				for (unsigned int i = 0, currentIndex = 0; i < text_length; ++i)
				{
					fragment.push_back(text[i]);

					if (amount && currentIndex < amount && i == indices[currentIndex])
					{
						++currentIndex;
						fragment.pop_back();

						ImVec2 fragment_text_size = ImGui::CalcTextSize(fragment.c_str());
						fragment_text_size.x += margin;
						if (line_length + fragment_text_size.x < ImGui::GetWindowWidth())
						{
							ImGui::SameLine(0.0f, 0.0f);
							line_length += fragment_text_size.x;
							margin = 0;
						}
						else {
							line_length = fragment_text_size.x;
							margin = NEWLINE_MARGIN;
						}

						ImGui::Text(fragment.c_str());
						fragment.clear();


						unsigned int cursor = i;
						while (currentIndex < amount && cursor + sample_length >= indices[currentIndex])
							cursor = indices[currentIndex++];
						while (i < cursor + sample_length)
							fragment.push_back(text[i++]);
						if (!fragment.length())
						{
							for (unsigned int j = 0; j < sample_length; ++j)
								fragment.push_back(sample[j]);
							i += sample_length;
						}
						--i;


						fragment_text_size = ImGui::CalcTextSize(fragment.c_str());
						fragment_text_size.x += margin;
						if (line_length + fragment_text_size.x < ImGui::GetWindowWidth())
						{
							ImGui::SameLine(0.0f, 0.0f);
							line_length += fragment_text_size.x;
							margin = 0;
						}
						else {
							line_length = fragment_text_size.x;
							margin = NEWLINE_MARGIN;
						}

						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), fragment.c_str());
						fragment.clear();
					}
					else if (text[i] == ' ' || i + 1 == text_length)
					{
						ImVec2 fragment_text_size = ImGui::CalcTextSize(fragment.c_str());
						fragment_text_size.x += margin;
						if (line_length + fragment_text_size.x < ImGui::GetWindowWidth())
						{
							ImGui::SameLine(0.0f, 0.0f);
							line_length += fragment_text_size.x;
							margin = 0;
						}
						else {
							line_length = fragment_text_size.x;
							margin = NEWLINE_MARGIN;
						}

						ImGui::Text(fragment.c_str());
						fragment.clear();
					}
				}
			}

			ImGui::End();
		}



		// std::string's that will store text and sample read from files
		static std::string text_from_file, sample_from_file;

		static std::vector<unsigned int> indices_from_file;

		static unsigned int text_from_file_length = 0, sample_from_file_length = 0, amount_from_file = 0;

		//static const double file_buttons_padding
		//	= ImGui::CalcTextSize(" \"string.txt\" ").x - ImGui::CalcTextSize(" \"text.txt\" ").x;

		// File input/output window
		if (SHOW_FILE)
		{
			ImGui::Begin("IO File", &SHOW_FILE);

			// Scale font size of resolution height
			ImGui::SetWindowFontScale(static_cast<float>(mode->height) / FONT_SCALE);
			


			// Flags to know when files are unavailable to read
			static bool file_opened_txt = true;
			static bool file_opened_str = true;
			static bool file_opened_out = true;
			static bool unexpected_error = false;

			// Time of search for file read texts
			static std::pair<double, double> file_time_search = { 0, 0 };



			ImGui::Text("Read from files:");

			ImGui::SameLine();

			HelpMarker("(?)", ".txt files must be located in\n the same folder with .exe file");
			
			if (ImGui::Button(" Open ->##0") )
			{
				if (std::filesystem::exists("text.txt") )
					//system("text.txt");
					ShellExecuteA(NULL, "open", "text.txt", NULL, NULL, SW_RESTORE);
				else
					file_opened_txt = false;
			}

			ImGui::SameLine(0.0f, 2.0f);

			// Read from text.txt and print length
			//static const double button_width_string = ImGui::CalcItemWidth();
			if (ImGui::Button(" \"text.txt\" ") )//, ImVec2(button_width_string, 0.0f)
			{
				std::ifstream file_txt("text.txt");

				file_opened_txt = file_txt.is_open();

				if (file_opened_txt)
				{
					std::stringstream strStream;
					strStream << file_txt.rdbuf();
					text_from_file = strStream.str();
				}

				text_from_file_length = text_from_file.length();

				file_txt.close();
			}
			ImGui::SameLine();
			if (file_opened_txt)
			{
				ImGui::Text(text_from_file_length ? "%zu Bytes" : "", text_from_file.length() );
			}
			else {
				text_from_file_length = 0;
				text_from_file.clear();
				ImGui::Text("Error: file not found!");
			}



			ImGui::Spacing();

			if (ImGui::Button(" Open ->##1") )
			{
				//system("string.txt");
				if (std::filesystem::exists("string.txt") )
					ShellExecuteA(NULL, "open", "string.txt", NULL, NULL, SW_RESTORE);
				else
					file_opened_str = false;
			}

			ImGui::SameLine(0.0f, 2.0f);

			// Read from string.txt and print length
			if (ImGui::Button(" \"string.txt\" ") )
			{
				std::ifstream file_str("string.txt");

				file_opened_str = file_str.is_open();

				if (file_opened_str)
				{
					std::stringstream strStream;
					strStream << file_str.rdbuf();
					sample_from_file = strStream.str();
				}

				sample_from_file_length = sample_from_file.length();

				file_str.close();
			}
			ImGui::SameLine();
			if (file_opened_str)
			{
				ImGui::Text(sample_from_file_length ? "%zu Bytes" : "", sample_from_file.length());
			}
			else {
				sample_from_file_length = 0;
				sample_from_file.clear();
				ImGui::Text("Error: file not found!");
			}



			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// ImGui::Combo to choose algorithm
			ImGui::Text("Choose algorithm:");
			ImGui::SameLine();
			HelpMarker("(*)", "Horspool algorithm works\nwith ASCII symbols only");

			ImGui::SetNextItemWidth(ImGui::CalcTextSize(ALGORITHMS[1]).x + 37);
			static int combo_item_current_idx = 0; // Here we store our selection data as an index.
			const char* combo_preview_value = ALGORITHMS[combo_item_current_idx];  // value visible before opening the combo
			if (ImGui::BeginCombo("##Combo", combo_preview_value) )
			{
				for (unsigned int n = 0; n < IM_ARRAYSIZE(ALGORITHMS); ++n)
				{
					const bool is_selected = (combo_item_current_idx == n);
					if (ImGui::Selectable(ALGORITHMS[n], is_selected))
						combo_item_current_idx = n;
					
					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}



			ImGui::Spacing();

			ImGui::Text("Print results in file:");

			if (ImGui::Button(" Open ->##2") )
			{
				//system("output.txt");
				//WinExec("output.txt");
				//WinExec("output.txt", SW_HIDE);
				if (std::filesystem::exists("output.txt"))
					ShellExecuteA(NULL, "open", "output.txt", NULL, NULL, SW_RESTORE);
				else
					file_opened_out = false;
			}

			ImGui::SameLine(0.0f, 2.0f);

			// Print indices vector and time in output.txt
			if (ImGui::Button(" \"output.txt\" "))
			{
				indices_from_file.clear();

				std::ofstream outfile("output.txt");
				
				file_opened_out = outfile.is_open();

				if (file_opened_out)
				{
					unsigned int memory_used = 2 * sizeof(unsigned int);

					if (CASE_SENSETIVE)
					{
						switch (combo_item_current_idx)
						{
						case 0:// Naive
							file_time_search = naive_search(text_from_file, sample_from_file, indices_from_file);
							memory_used += 2 * sizeof(unsigned int);
							break;
						case 1:// Knuth-Morris-Pratt
							file_time_search = kmp_search(text_from_file, sample_from_file, indices_from_file);
							if (file_time_search.second)
								memory_used += (2 + sample_from_file_length) * sizeof(unsigned int);
							break;
						case 2:// Colussi
							file_time_search = colussi_search(text_from_file, sample_from_file, indices_from_file);
							if (file_time_search.second)
								memory_used += (9 + 6 * sample_from_file_length) * sizeof(unsigned int);
							break;
						case 3:// Horspool
							file_time_search = horspool_search(text_from_file, sample_from_file, indices_from_file);
							if (file_time_search.second)
								memory_used += 258 * sizeof(unsigned int) + sizeof(unsigned char);
							break;
						case 4:// Rabin-Karp
							file_time_search = rabin_carp_search(text_from_file, sample_from_file, indices_from_file);
							if (file_time_search.second)
								memory_used += 5 * sizeof(unsigned int);
							break;
						default:
							unexpected_error = true;
							file_opened_out = false;
							break;
						}
					}
					else {
						text_upper = boost::to_upper_copy<std::string>(text_from_file);
						sample_upper = boost::to_upper_copy<std::string>(sample_from_file);

						memory_used += text_upper.length() + sample_upper.length();

						switch (combo_item_current_idx)
						{
						case 0:// Naive
							file_time_search = naive_search(text_upper, sample_upper, indices_from_file);
							memory_used += 2 * sizeof(unsigned int);
							break;
						case 1:// Knuth-Morris-Pratt
							file_time_search = kmp_search(text_upper, sample_upper, indices_from_file);
							if (file_time_search.second)
								memory_used += (2 + sample_from_file_length) * sizeof(unsigned int);
							break;
						case 2:// Colussi
							file_time_search = colussi_search(text_upper, sample_upper, indices_from_file);
							if (file_time_search.second)
								memory_used += (9 + 6 * sample_from_file_length) * sizeof(unsigned int);
							break;
						case 3:// Horspool
							file_time_search = horspool_search(text_upper, sample_upper, indices_from_file);
							if (file_time_search.second)
								memory_used += 258 * sizeof(unsigned int) + sizeof(unsigned char);
							break;
						case 4:// Rabin-Karp
							file_time_search = rabin_carp_search(text_upper, sample_upper, indices_from_file);
							if (file_time_search.second)
								memory_used += 5 * sizeof(unsigned int);
							break;
						default:
							unexpected_error = true;
							file_opened_out = false;
							break;
						}
					}
					
					amount_from_file = indices_from_file.size();

					if (amount_from_file)
					{
						outfile << "Algorithm used: " << ALGORITHMS[combo_item_current_idx]
							<< ";\nSearch finished in\n " << file_time_search.first << "s preprocessing;\n "
							<< file_time_search.second << "s search;\n "
							<< file_time_search.first + file_time_search.second << "s total;\n"
							<< "Memory used: " << memory_used << " Bytes;\nFound "
							<< amount_from_file << (amount_from_file == 1 ? " match" : " matches") << ":\n\n";
						std::ostream_iterator<unsigned int> output_iterator(outfile, "\n");
						std::copy(indices_from_file.begin(), indices_from_file.end(), output_iterator);
					}
					else {
						outfile << "Algorithm used: " << ALGORITHMS[combo_item_current_idx]
							<< ";\nSearch finished in\n " << file_time_search.first << "s preprocessing;\n "
							<< file_time_search.second << "s search;\n "
							<< file_time_search.first + file_time_search.second << "s total;\n"
							<< "Memory used: " << memory_used << " Bytes;\nNo matches found.";
					}
				}
				
				outfile.close();
			}
			ImGui::SameLine();
			if (file_opened_out)
			{
				ImGui::Text(file_time_search.first + file_time_search.second ? "%.7fs taken" : "", file_time_search.first + file_time_search.second);
			}
			else if (unexpected_error)
			{
				ImGui::Text("Something seems wrong :(");
			}
			else {
				ImGui::Text("Error: file not found!");
			}



			ImGui::Spacing();
			ImGui::Spacing();
			
			static bool file_demo_output = false;
			/*
			if (ImGui::Button(" Demo Output "))
			{
				file_demo_output = true;
			}*/
			
			ImGui::Checkbox("Show Demo Output", &file_demo_output);

			if (file_demo_output)
			{
				ImGui::Begin("File Demo Output", &file_demo_output, ImGuiWindowFlags_HorizontalScrollbar);

				// Scale font size of resolution height
				ImGui::SetWindowFontScale(static_cast<float>(mode->height) / FONT_SCALE);

				std::string fragment;
				
				float margin = NEWLINE_MARGIN, line_length = 0;

				for (unsigned int i = 0, currentIndex = 0; i < text_from_file_length; ++i)
				{
					fragment.push_back(text_from_file[i]);

					if (amount_from_file && currentIndex < amount_from_file && i == indices_from_file[currentIndex])
					{
						++currentIndex;
						fragment.pop_back();

						ImVec2 fragment_text_size = ImGui::CalcTextSize(fragment.c_str());
						fragment_text_size.x += margin;
						if (line_length + fragment_text_size.x < ImGui::GetWindowWidth())
						{
							ImGui::SameLine(0.0f, 0.0f);
							line_length += fragment_text_size.x;
							margin = 0;
						}
						else {
							line_length = fragment_text_size.x;
							margin = NEWLINE_MARGIN;
						}

						ImGui::Text(fragment.c_str());
						fragment.clear();


						unsigned int cursor = i;
						while (currentIndex < amount_from_file && cursor + sample_from_file_length >= indices_from_file[currentIndex])
							cursor = indices_from_file[currentIndex++];
						while (i < cursor + sample_from_file_length)
							fragment.push_back(text_from_file[i++]);
						if (!fragment.length())
						{
							for (unsigned int j = 0; j < sample_from_file_length; ++j)
								fragment.push_back(sample_from_file[j]);
							i += sample_from_file_length;
						}
						--i;


						fragment_text_size = ImGui::CalcTextSize(fragment.c_str());
						fragment_text_size.x += margin;
						if (line_length + fragment_text_size.x < ImGui::GetWindowWidth())
						{
							ImGui::SameLine(0.0f, 0.0f);
							line_length += fragment_text_size.x;
							margin = 0;
						}
						else {
							line_length = fragment_text_size.x;
							margin = NEWLINE_MARGIN;
						}

						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), fragment.c_str());
						fragment.clear();
					}
					else if (text_from_file[i] == ' ' || i + 1 == text_from_file_length || text_from_file[i] == '\n')
					{
						ImVec2 fragment_text_size = ImGui::CalcTextSize(fragment.c_str());
						fragment_text_size.x += margin;
						if (line_length + fragment_text_size.x < ImGui::GetWindowWidth())
						{
							ImGui::SameLine(0.0f, 0.0f);
							line_length += fragment_text_size.x;
							margin = 0;
						}
						else {
							line_length = fragment_text_size.x;
							margin = NEWLINE_MARGIN;
						}

						ImGui::Text(fragment.c_str());
						fragment.clear();
					}
				}

				ImGui::End();
			}



			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Clear RAM:");

			// clear both text and sample
			if (ImGui::Button(" Reset "))
			{
				file_opened_txt = file_opened_str = file_opened_out = true;
				unexpected_error = false;
				text_from_file_length = sample_from_file_length = amount_from_file = 0;
				file_time_search = { 0, 0 };
				text_from_file.clear();
				sample_from_file.clear();
				indices_from_file.clear();
			}

			/*
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text(text_from_file.c_str() );
			ImGui::Text(sample_from_file.c_str() );
			*/

			ImGui::End();
		}



		if (SHOW_CHRONO)
		{
			ImGui::Begin("Chrono table", &SHOW_CHRONO);

			// Scale font size of resolution height
			ImGui::SetWindowFontScale(static_cast<float>(mode->height) / FONT_SCALE);



			static bool chrono_demo_mode = true;
			ImGui::Text("Current mode:");
			ImGui::SameLine();
			ToggleButton("text", &chrono_demo_mode);
			ImGui::SameLine();
			ImGui::Text(chrono_demo_mode ? "Demo" : "File");



			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// Input amount of measurements, >= 1
			static int number_of_measurements_input = 100, number_of_measurements = 100;
			static const double int_input_width = ImGui::CalcTextSize("1000000000").x * 1.7;



			ImGui::Spacing();
			ImGui::Spacing();

			static bool is_active[IM_ARRAYSIZE(ALGORITHMS)] = { true, true, true, true, true };

			static std::pair<double, double> chrono_pair[IM_ARRAYSIZE(ALGORITHMS)] = { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };
			
			if (ImGui::BeginTable("myTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX) )
			{
				ImGui::TableSetupColumn("Algorithm");
				ImGui::TableSetupColumn("Precalc-s, s", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Search, s", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Total, s", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableHeadersRow();
				
				for (int row = 0; row < IM_ARRAYSIZE(ALGORITHMS); row++)
				{
					/*
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Hello");
					for (int column = 1; column < 3; column++)
					{
						ImGui::TableSetColumnIndex(column);
						char buf[32];
						sprintf(buf, "Hello %d,%d", column, row);
						ImGui::TextUnformatted(buf);
					}
					ImGui::TableNextColumn();
					ImGui::Text("Bye!");
					*/

					ImGui::TableNextRow();
					
					ImGui::TableNextColumn();
					//ImGui::TextUnformatted(ALGORITHMS[row]);
					ImGui::Checkbox(ALGORITHMS[row], &is_active[row]);

					if (is_active[row])
					{
						ImGui::TableNextColumn();
						ImGui::Text("%.9f", chrono_pair[row].first / number_of_measurements);

						ImGui::TableNextColumn();
						ImGui::Text("%.9f", chrono_pair[row].second / number_of_measurements);

						ImGui::TableNextColumn();
						ImGui::Text("%.9f", (chrono_pair[row].first + chrono_pair[row].second) / number_of_measurements);
					}
					else {
						for (int column = 1; column < 4; column++)
							ImGui::TableNextColumn();
					}
				}

				ImGui::EndTable();
			}



			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SetNextItemWidth(int_input_width);
			if (ImGui::InputInt("Number of measurements", &number_of_measurements_input, 10) && number_of_measurements_input < 1)
				number_of_measurements_input = 1;



			ImGui::Spacing();
			ImGui::Spacing();

			enum Algos {// NOTE: do not forget to change according to any ALGORITHMS changes
				Naive,
				K_m_p,
				Clssi,
				Hrspl,
				RKarp,
			};

			if (ImGui::Button("  Start  ") )
			{
				number_of_measurements = number_of_measurements_input;

				std::vector<unsigned int> indices_chrono;

				for (int i = 0; i < IM_ARRAYSIZE(ALGORITHMS); ++i)
					chrono_pair[i] = { 0, 0 };
				
				if (CASE_SENSETIVE)
				{
					if (chrono_demo_mode)
					{
						for (int i = 0; i < number_of_measurements; ++i)
						{
							/*
							for (int j = 0; j < IM_ARRAYSIZE(ALGORITHMS); ++j)
							{
								if (is_active[j])
								{
									chrono_pair[j] +=
								}
							}
							*/
							if (is_active[Algos::Naive])
								chrono_pair[Algos::Naive] += naive_search(text, sample, indices_chrono);

							if (is_active[Algos::K_m_p])
								chrono_pair[Algos::K_m_p] += kmp_search(text, sample, indices_chrono);

							if (is_active[Algos::Clssi])
								chrono_pair[Algos::Clssi] += colussi_search(text, sample, indices_chrono);

							if (is_active[Algos::Hrspl])
								chrono_pair[Algos::Hrspl] += horspool_search(text, sample, indices_chrono);

							if (is_active[Algos::RKarp])
								chrono_pair[Algos::RKarp] += rabin_carp_search(text, sample, indices_chrono);
						}
					}
					else {
						for (int i = 0; i < number_of_measurements; ++i)
						{
							if (is_active[Algos::Naive])
								chrono_pair[Algos::Naive] += naive_search(text_from_file, sample_from_file, indices_chrono);

							if (is_active[Algos::K_m_p])
								chrono_pair[Algos::K_m_p] += kmp_search(text_from_file, sample_from_file, indices_chrono);

							if (is_active[Algos::Clssi])
								chrono_pair[Algos::Clssi] += colussi_search(text_from_file, sample_from_file, indices_chrono);

							if (is_active[Algos::Hrspl])
								chrono_pair[Algos::Hrspl] += horspool_search(text_from_file, sample_from_file, indices_chrono);

							if (is_active[Algos::RKarp])
								chrono_pair[Algos::RKarp] += rabin_carp_search(text_from_file, sample_from_file, indices_chrono);
						}
					}
				}
				else {
					if (chrono_demo_mode)
					{
						text_upper = boost::to_upper_copy<std::string>(text);
						sample_upper = boost::to_upper_copy<std::string>(sample);

						for (int i = 0; i < number_of_measurements; ++i)
						{
							if (is_active[Algos::Naive])
								chrono_pair[Algos::Naive] += naive_search(text_upper, sample_upper, indices_chrono);

							if (is_active[Algos::K_m_p])
								chrono_pair[Algos::K_m_p] += kmp_search(text_upper, sample_upper, indices_chrono);

							if (is_active[Algos::Clssi])
								chrono_pair[Algos::Clssi] += colussi_search(text_upper, sample_upper, indices_chrono);

							if (is_active[Algos::Hrspl])
								chrono_pair[Algos::Hrspl] += horspool_search(text_upper, sample_upper, indices_chrono);

							if (is_active[Algos::RKarp])
								chrono_pair[Algos::RKarp] += rabin_carp_search(text_upper, sample_upper, indices_chrono);
						}
					}
					else {
						text_upper = boost::to_upper_copy<std::string>(text_from_file);
						sample_upper = boost::to_upper_copy<std::string>(sample_from_file);

						for (int i = 0; i < number_of_measurements; ++i)
						{
							if (is_active[Algos::Naive])
								chrono_pair[Algos::Naive] += naive_search(text_upper, sample_upper, indices_chrono);

							if (is_active[Algos::K_m_p])
								chrono_pair[Algos::K_m_p] += kmp_search(text_upper, sample_upper, indices_chrono);

							if (is_active[Algos::Clssi])
								chrono_pair[Algos::Clssi] += colussi_search(text_upper, sample_upper, indices_chrono);

							if (is_active[Algos::Hrspl])
								chrono_pair[Algos::Hrspl] += horspool_search(text_upper, sample_upper, indices_chrono);

							if (is_active[Algos::RKarp])
								chrono_pair[Algos::RKarp] += rabin_carp_search(text_upper, sample_upper, indices_chrono);
						}
					}
				}
			}

			ImGui::SameLine();

			HelpMarker("(*)", "Horspool algorithm works\nwith ASCII symbols sonly");



			ImGui::End();
		}



		//ImGui::ShowDemoWindow();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Substring Search";
	spec.Height = 800;
	spec.Width = 1400;

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
					app->Close();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Options"))
			{
				ImGui::Checkbox("Case sensitive", &CASE_SENSETIVE);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::MenuItem("Demo Input/Output", "", SHOW_DEMO))
				{
					SHOW_DEMO = (SHOW_DEMO ? false : true);
				}
				if (ImGui::MenuItem("File Input/Output", "", SHOW_FILE))
				{
					SHOW_FILE = (SHOW_FILE ? false : true);
				}
				if (ImGui::MenuItem("Time measurement", "", SHOW_CHRONO))
				{
					SHOW_CHRONO = (SHOW_CHRONO ? false : true);
				}
				ImGui::EndMenu();
			}
		});
	return app;
}
