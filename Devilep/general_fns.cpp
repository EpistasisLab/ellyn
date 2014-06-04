#include "stdafx.h"
//#include "pop.h"
#include "params.h"

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && (std::isdigit(*it) || (*it=='-') || (*it=='.'))) ++it;
    return !s.empty() && it == s.end();
}

void NewInstruction(ind& newind,int loc,params& p,vector<Randclass>& r,data& d)
{
	
	vector <string> load_choices(p.allblocks);
	int choice=0;
	//cout<<"iterator definition\n";
	//std::vector<int>::iterator it;
	//it = newind.line.begin();
	//cout<<"end def \n";

	//uniform_int_distribution<int> dist(0,21);
	if (p.ERC)
	{
		float ERC;
		for (int j=0;j<p.numERC;j++)
		{
			ERC = r[omp_get_thread_num()].rnd_flt((float)p.minERC,(float)p.maxERC);
			if(p.ERCints)
				load_choices.push_back(to_string(static_cast<long long>(ERC)));
			else
				load_choices.push_back(to_string(static_cast<long double>(ERC)));
		}
	}
	vector<float> wheel(p.op_weight.size());
	//wheel.resize(p.op_weight.size());
	if (p.weight_ops_on) //fns are weighted
	{
		partial_sum(p.op_weight.begin(), p.op_weight.end(), wheel.begin());
	}
	
	if (p.weight_ops_on) //fns are weighted
	{
		float tmp = r[omp_get_thread_num()].rnd_flt(0,1);
		if (tmp < wheel.at(0))
			choice=0;
		else
		{
			for (unsigned int k=1;k<wheel.size();k++)
			{
				if(tmp<wheel.at(k) && tmp>=wheel.at(k-1))
					choice = k;
			}
		}
	}
	else 
		choice = r[omp_get_thread_num()].rnd_int(0,p.op_list.size()-1);

		string varchoice;
			
		switch (p.op_choice.at(choice))
		{
		case 0: //load number
			/*if(p.ERCints)
				newind.line.at(loc)=shared_ptr<node>(new n_num((float)r[omp_get_thread_num()].rnd_int(p.minERC,p.maxERC)));
			else
				newind.line.at(loc)=shared_ptr<node>(new n_num(r[omp_get_thread_num()].rnd_flt(p.minERC,p.maxERC)));
			break;*/
			if(p.ERC){ // if ephemeral random constants are on
				if (!p.cvals.empty()){
					if (r[omp_get_thread_num()].rnd_flt(0,1)<.5)
						newind.line.push_back(shared_ptr<node>(new n_num(p.cvals.at(r[omp_get_thread_num()].rnd_int(0,p.cvals.size()-1)))));
					else{	
						if(p.ERCints)
							newind.line.push_back(shared_ptr<node>(new n_num((float)r[omp_get_thread_num()].rnd_int(p.minERC,p.maxERC))));
						else
							newind.line.push_back(shared_ptr<node>(new n_num(r[omp_get_thread_num()].rnd_flt(p.minERC,p.maxERC))));
					}
				}
				else{
					if(p.ERCints)
							newind.line.push_back(shared_ptr<node>(new n_num((float)r[omp_get_thread_num()].rnd_int(p.minERC,p.maxERC))));
						else
							newind.line.push_back(shared_ptr<node>(new n_num(r[omp_get_thread_num()].rnd_flt(p.minERC,p.maxERC))));
				}
			}
			else if (!p.cvals.empty())
			{
				newind.line.push_back(shared_ptr<node>(new n_num(p.cvals.at(r[omp_get_thread_num()].rnd_int(0,p.cvals.size()-1)))));
			}
			break;
		case 1: //load variable
			varchoice = d.label.at(r[omp_get_thread_num()].rnd_int(0,d.label.size()-1));
			newind.line.at(loc)=shared_ptr<node>(new n_sym(varchoice));
			break;
		case 2: // +
			newind.line.at(loc)=shared_ptr<node>(new n_add());
			break;
		case 3: // -
			newind.line.at(loc)=shared_ptr<node>(new n_sub());
			break;
		case 4: // *
			newind.line.at(loc)=shared_ptr<node>(new n_mul());
			break;
		case 5: // /
			newind.line.at(loc)=shared_ptr<node>(new n_div());
			break;
		case 6: // sin
			newind.line.at(loc)=shared_ptr<node>(new n_sin());
			break;
		case 7: // cos
			newind.line.at(loc)=shared_ptr<node>(new n_cos());
			break;
		case 8: // exp
			newind.line.at(loc)=shared_ptr<node>(new n_exp());
			break;
		case 9: // log
			newind.line.at(loc)=shared_ptr<node>(new n_log());
			break;
		}
		

}

void makenewcopy(ind& newind)
{
	for (int i=0;i<newind.line.size();i++)
	{
		if (newind.line.at(i).use_count()>1)
		{
			string varname;
			float value;
			bool onval;
			switch (newind.line.at(i)->type){
			case 'n':
				value = static_pointer_cast<n_num>(newind.line.at(i))->value;
				onval = newind.line.at(i)->on;
				newind.line.at(i) = shared_ptr<node>(new n_num(value));
				newind.line.at(i)->on=onval;
				break;
			case 'v':
				varname = static_pointer_cast<n_sym>(newind.line.at(i))->varname;
				onval = newind.line.at(i)->on;
				newind.line.at(i) = shared_ptr<node>(new n_sym(varname));
				newind.line.at(i)->on=onval;
				break;
			case '+': // +
				onval = newind.line.at(i)->on;
				newind.line.at(i)=shared_ptr<node>(new n_add());
				newind.line.at(i)->on=onval;
				break;
			case '-': // -
				onval = newind.line.at(i)->on;
				newind.line.at(i)=shared_ptr<node>(new n_sub());
				newind.line.at(i)->on=onval;
				break;
			case '*': // *
				onval = newind.line.at(i)->on;
				newind.line.at(i)=shared_ptr<node>(new n_mul());
				newind.line.at(i)->on=onval;
				break;
			case '/': // /
				onval = newind.line.at(i)->on;
				newind.line.at(i)=shared_ptr<node>(new n_div());
				newind.line.at(i)->on=onval;
				break;
			case 's': // sin
				onval = newind.line.at(i)->on;
				newind.line.at(i)=shared_ptr<node>(new n_sin());
				newind.line.at(i)->on=onval;
				break;
			case 'c': // cos
				onval = newind.line.at(i)->on;
				newind.line.at(i)=shared_ptr<node>(new n_cos());
				newind.line.at(i)->on=onval;
				break;
			case 'e': // exp
				onval = newind.line.at(i)->on;
				newind.line.at(i)=shared_ptr<node>(new n_exp());
				newind.line.at(i)->on=onval;
				break;
			case 'l': // log
				onval = newind.line.at(i)->on;
				newind.line.at(i)=shared_ptr<node>(new n_log());
				newind.line.at(i)->on=onval;
				break;
				}
		}
		else if (newind.line.at(i).use_count()==0)
		{
			cerr << "shared pointer use count is zero\n";
		}
}
}