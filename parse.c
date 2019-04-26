#include "wottocc.h"

void program() {
	while (((Token *)vec_get(tokens, pos))->ty != TK_EOF) {
		//Map *variables = new_map();
		//vec_push(genv, variables);
		vec_push(functions, function());
		envnum++;
	}
	return;
}

Node *function() {
	Node *node;

	err_consume(TK_INT, "It's not suitable format for function");
	err_consume(TK_IDENT, "It's not suitable format for function");
	err_consume('(', "no left-parenthesis at declaration_list");

	node = new_node(ND_FUNCDEF, NULL, new_env(NULL), NULL, NULL);
	node->fname = ((Token *)vec_get(tokens, pos-2))->input;
	//map_put(node->env->variables, node->fname, 0, NULL);
	Vector *args = new_vector();
	// int foo(int *x, int y){ ... }
	//Map *variables = vec_get(genv, envnum);
	while (1) {
		if (consume(')')) {
			break;
		}
		err_consume(TK_INT, "no type at args");
		Type *type = new_type(TY_INT);

		while (consume('*')) {
			Type *newtype = new_type(TY_PTR);
			newtype->ptrof = type;
			type = newtype;
		}
		err_consume(TK_IDENT, "args is not variable");

		char *vname = ((Token *)vec_get(tokens,pos-1))->input;	
		map_put(node->env->variables, vname, 0, type);

		Node *arg = new_node(ND_INT, type, NULL, NULL, NULL);
		arg->name = vname;
		vec_push(args, arg);

		if (!consume(',')) {
			err_consume(')', "there's no right-parenthesis at args");
			break;
		}

	}
	node->args = args;

	node->lhs = compound_statement(node->env);
	return node;
}

Node *compound_statement(Env *env) {
	Node *node = new_node(ND_COMPOUND_STMT, NULL, env, NULL, NULL);
	err_consume('{', "no left-brace at compound_statement");
	//if(!consume('}')){
		Env *inner_env = new_env(env);
		inner_env->my_switch_cnt = env->my_switch_cnt;
		inner_env->my_loop_cnt = env->my_loop_cnt;
		node->lhs = block_item_list(inner_env);
		vec_push(env->inner, inner_env);
	//} else {
	//	return node;
	//}
	err_consume('}', "no right-brace at compound_statement");
	return node;
}

Node *block_item_list(Env *env) {
	Node *node = new_node(ND_BLOCKITEMLIST, NULL, env, NULL, NULL);
	Vector *args = new_vector();
	while (1) {
		Node *arg = block_item(env);
		if (arg == NULL) break;
		else vec_push(args, arg);
	}
	node->args = args;
	return node;
}

Node *block_item(Env *env) {
	Node *node = declaration(env);
	if (node == NULL){
		node = statement(env);
	}
	return node;
}

Node *declaration(Env *env) {
	Node *node = NULL;
	if (consume(TK_INT)) {
		node = new_node(ND_DECLARATION, NULL, env, NULL, NULL);
		Type *type = new_type(TY_INT); 

		if (read_nextToken(';') == 0) {
			node->lhs = init_declarator_list(env, type);
		}

		err_consume(';', "no ';' at declaration");
	}
	return node;
}

Node *init_declarator_list(Env *env, Type *sp_type) {
	Node *node = new_node(ND_INIT_DECLARATOR_LIST, NULL, env, NULL, NULL);
	Vector *args = new_vector();
	Node *arg = init_declarator(env, sp_type);
	vec_push(args, arg);
	while(1) {
		if (!consume(',')) break;
		Node *arg = init_declarator(env, sp_type);
		vec_push(args, arg);
	}
	node->args = args;
	return node;
}	

Node *init_declarator(Env *env, Type *sp_type) {
	Node *node = declarator(env, sp_type);

	if (consume('=')) {
		node = new_node(ND_INIT_DECLARATOR, sp_type, env, node, initializer(env));
	}
	return node;
}

Node *initializer(Env *env) {
	Node *node = assignment_expression(env);
	return node;
}

Node *declarator(Env *env, Type *sp_type) {
	Type *type = sp_type;

	while (consume('*')) {
		Type *newtype = new_type(TY_PTR);
		newtype->ptrof = type;
		type = newtype;
	}

	err_consume(TK_IDENT, "no identifier at declarator");
	char *vname = ((Token *)vec_get(tokens,pos-1))->input;	

	if (consume('[')) {
		err_consume(TK_NUM, "array initializer must be number");
		Type *newtype = new_type(TY_ARRAY);
		newtype->ptrof = type;
		newtype->array_size = ((Token *)vec_get(tokens,pos-1))->val;
		type = newtype;
		err_consume(']', "no ']' at array-def");
	}
	
	map_put(env->variables, vname, 0, type);
	
	Node *node = new_node(ND_IDENT, type, env, NULL, NULL);
	node->name = vname;

	return node;
}



Node *statement(Env *env) {
	Node *node = NULL;
	if (read_nextToken(TK_CASE)) {
		node = labeled_statement(env);
	} else if (read_nextToken(TK_RETURN) || read_nextToken(TK_BREAK) || read_nextToken(TK_CONTINUE)) {
		node = jump_statement(env);
	} else if (read_nextToken(TK_IF) || read_nextToken(TK_SWITCH)) {
		node = selection_statement(env);
	} else if (read_nextToken(TK_WHILE) || read_nextToken(TK_FOR)) {
		node = iteration_statement(env);
	} else if (read_nextToken('{')) {
		node = compound_statement(env);
	} else {
		node = expression_statement(env);
	}
	return node;
}

Node *jump_statement(Env *env) {
	Node *node = NULL;
	if (consume(TK_RETURN)) {
		node = new_node(ND_RETURN, NULL, env, expression(env), NULL);
		err_consume(';', "no ';' at return");
	} else if (consume(TK_BREAK)) {
		node = new_node(ND_BREAK, NULL, env, NULL, NULL);
		err_consume(';', "no ';' at break");
	} else if (consume(TK_CONTINUE)) {
		node = new_node(ND_CONTINUE, NULL, env, NULL, NULL);
		err_consume(';', "no ';' at continue");
	} 
	return node;
}

Node *expression_statement(Env *env) {
	Node *node = new_node(ND_EXPRESSION_STMT, NULL, env, expression(env), NULL);
	if (node->lhs == NULL) {
		if (!consume(';')) {
			return NULL;
		}
	} else {
		err_consume(';', "no ';' at end of the expression_statement\n");
	}
	return node;
}

Node *selection_statement(Env *env) {
	Node *node;
	if (consume(TK_IF)) {
		node = new_node(ND_IF, NULL, env, NULL, NULL);
		err_consume('(', "no left-parenthesis at if");
		Vector *arg = new_vector();
		vec_push(arg, expression(env));
		node->args = arg;
		err_consume(')', "no right-parenthesis at if");
		node->lhs = statement(env);
		if (consume(TK_ELSE)) {
			node->rhs = statement(env);
		} else {
			node->rhs = NULL;
		}
	} else if (consume(TK_SWITCH)) {
		err_consume('(', "no left-parenthesis at switch");
		node = new_node(ND_SWITCH, NULL, env, expression(env), NULL);
		err_consume(')', "no right-parenthesis at switch");
		Env *inner_env = new_env(env);
		switch_loop_cnt++;
		inner_env->my_switch_cnt = switch_loop_cnt;
		inner_env->my_loop_cnt = while_loop_cnt;
		node->rhs = statement(inner_env);
		vec_push(env->inner, inner_env);
	}

	return node;
}

Node *labeled_statement(Env *env) {
	Node *node = NULL;
	if (consume(TK_CASE)) {
		node = new_node(ND_CASE, NULL, env, NULL, NULL);
		node->val = env->cases->len;
		vec_push(env->cases, equality_expression(env));
		err_consume(':', "no colon at case");
		node->rhs = statement(env);
	}
	return node;
}


Node *iteration_statement(Env *env) {
	Node *node;
	if (consume(TK_WHILE)) {
		node = new_node(ND_WHILE, NULL, env, NULL, NULL);
		err_consume('(', "no left-parenthesis at while");
		Vector *arg = new_vector();
		vec_push(arg, expression(env));
		node->args = arg;
		err_consume(')', "no right-parenthesis at while");
		Env *inner_env = new_env(env);
		switch_loop_cnt++;
		while_loop_cnt = switch_loop_cnt;
		inner_env->my_switch_cnt = switch_loop_cnt;
		inner_env->my_loop_cnt = while_loop_cnt;
		node->lhs = statement(inner_env);
		vec_push(env->inner, inner_env);

	} else if (consume(TK_FOR)) {
		node = new_node(ND_FOR, NULL, env, NULL, NULL);
		err_consume('(', "no left-parenthesis at for");
		Vector *arg = new_vector();

		Node *tmp = expression(env);
		if (tmp != NULL) { 
			vec_push(arg, tmp);
			err_consume(';', "no ';' at for");
		} else { 
			vec_push(arg, declaration(env));
		}
		vec_push(arg, expression(env));
		err_consume(';', "no ';' at while");
		vec_push(arg, expression(env));
		node->args = arg;
		err_consume(')', "no right-parenthesis at for");
		Env *inner_env = new_env(env);
		switch_loop_cnt++;
		while_loop_cnt = switch_loop_cnt;
		inner_env->my_switch_cnt = switch_loop_cnt;
		inner_env->my_loop_cnt = while_loop_cnt;
		node->lhs = statement(inner_env);
		vec_push(env->inner, inner_env);
	}
	return node;
}

Node *expression(Env *env) {
	Node *node = assignment_expression(env);

	for (;;) {
		if (consume(',')) {
			node = new_node(ND_EXP, NULL, env, node, assignment_expression(env));
		} else {
			return node;
		}
	}
}


Node *assignment_expression(Env *env) {
	Node *node = conditional_expression(env); // analyzeで=の場合は左辺がunary_expressionであることを確認
	//Node *node = NULL;

	if (consume('=')){
		Node *lhs = node;
		Node *rhs = assignment_expression(env);
		Type *value_ty;
		if (lhs->value_ty->ty == TY_INT){
			if (rhs->value_ty->ty != TY_INT) {
				error("substitution from ptr to int: %s\n", ((Token *)vec_get(tokens, pos++))->input);
			}
			value_ty = new_type(TY_INT);
		} else {
			// ptr or array
			if (rhs->value_ty->ty == TY_INT) {
				error("substitution from int to ptr: %s\n", ((Token *)vec_get(tokens, pos++))->input);
			}
			value_ty = rhs->value_ty;
		}
		node = new_node('=', value_ty, env, lhs, rhs);
	}
	return node;
}

Node *conditional_expression(Env *env) {
	Node *node = logical_OR_expression(env);

	return node;
}

Node *logical_OR_expression(Env *env) {
	Node *node = logical_AND_expression(env);

	return node;
}

Node *logical_AND_expression(Env *env) {
	Node *node = inclusive_OR_expression(env);

	return node;
}

Node *inclusive_OR_expression(Env *env) {
	Node *node = exclusive_OR_expression(env);

	return node;
}

Node *exclusive_OR_expression(Env *env) {
	Node *node = AND_expression(env);

	return node;
}

Node *AND_expression(Env *env) {
	Node *node = equality_expression(env);

	return node;
}

Node *equality_expression(Env *env) {
	Node *node = relational_expression(env);

	for (;;) {
		if (consume(TK_EQUAL))
			node = new_node(ND_EQUAL, new_type(TY_INT), env, node, relational_expression(env));
		else if (consume(TK_NEQUAL))
			node = new_node(ND_NEQUAL, new_type(TY_INT), env, node, relational_expression(env));
		else
			return node;
	}
}

Node *relational_expression(Env *env) {
	Node *node = shift_expression(env);
	
	for (;;) {
		if (consume('<'))
			node = new_node('<', new_type(TY_INT), env, node, shift_expression(env));
		else if (consume('>'))
			node = new_node('>', new_type(TY_INT), env, node, shift_expression(env));
		else if (consume(TK_LEQ))
			node = new_node(ND_LEQ, new_type(TY_INT), env, node, shift_expression(env));
		else if (consume(TK_GEQ))
			node = new_node(ND_GEQ, new_type(TY_INT), env, node, shift_expression(env));
		else 
			return node;
	}
}

Node *shift_expression(Env *env) {
	Node *node = additive_expression(env);
	return node;
}

Node *additive_expression(Env *env) {
	Node *node = multiplicative_expression(env);

	for (;;) {
		if (consume('+')){
			Type *value_ty;
			Node *lhs = node;
			Node *rhs = multiplicative_expression(env);
			if (lhs->value_ty->ty != TY_INT || rhs->value_ty->ty != TY_INT) {
				// ptr
				if (lhs->value_ty->ty != TY_INT)
					value_ty = lhs->value_ty;
				else 
					value_ty = rhs->value_ty;
			} else {
				value_ty = new_type(TY_INT);
			}
			node = new_node('+', value_ty, env, lhs, rhs);
		} else if (consume('-')) {
			Node *lhs = node;
			Node *rhs = multiplicative_expression(env);
			Type *value_ty;
			if (lhs->value_ty->ty != TY_INT || rhs->value_ty->ty != TY_INT) {
				// ptr
				value_ty = new_type(TY_PTR);
			} else {
				value_ty = new_type(TY_INT);
			}
			node = new_node('-', value_ty, env, lhs, rhs);
		} else {
			return node;
		}
	}
}

Node *multiplicative_expression(Env *env) {
	Node *node = cast_expression(env);

	for (;;) {
		if (consume('*'))
			node = new_node('*', new_type(TY_INT), env, node, cast_expression(env));
		else if (consume('/'))
			node = new_node('/', new_type(TY_INT), env, node, cast_expression(env));
		else
			return node;
	}
}

Node *cast_expression(Env *env) {
	Node *node = unary_expression(env);
	return node;
}

Node *unary_expression(Env *env) {
	Node *node = NULL;

	if (consume(TK_PREINC)) {
		node = new_node(ND_PREINC, new_type(TY_INT), env, unary_expression(env), NULL);
	} else if (consume(TK_PREDEC)) {
		node = new_node(ND_PREDEC, new_type(TY_INT), env, unary_expression(env), NULL);
	} else if (consume('*')) {
		Node *lhs = cast_expression(env);
		if (lhs->value_ty->ty == TY_INT) {
			error("illegal deref: %s\n", ((Token *)vec_get(tokens, pos))->input);
		}
		node = new_node(ND_DEREF, new_type(lhs->value_ty->ptrof->ty), env, lhs, NULL);
	} else if (consume('&')) {
		node = new_node('&', new_type(TY_PTR), env, cast_expression(env), NULL);
	} else {
		node = postfix_expression(env);
	}
	return node;
}

Node *postfix_expression(Env *env) {
	Node *node = primary_expression(env);

	for(;;) {
		if (consume('[')) {
			// a[3] -> *(a + 3)
			Node *rhs = expression(env);
			Node *plus = new_node('+', new_type(TY_PTR), env, node, rhs);
			node = new_node(ND_DEREF, new_type(node->value_ty->ptrof->ty), env, plus, NULL);
			err_consume(']', "no right_braket at array");
		} else if (consume('(')) {
			// foo(1 ,2)
			node = new_node(ND_FUNC_CALL, NULL, env, node, argument_expression_list(env));
			err_consume(')', "no right-parenthesis at func_call");
		} else {
			return node;
		}
	}
}

Node *argument_expression_list(Env *env) {
	// 1, 2, 3
	Node *node = assignment_expression(env);
	int length = 1;
	if (node == NULL) return NULL;
	node->length = length;
	for (;;) {
		if (consume(',')) {
			node = new_node(ND_ARG_EXP_LIST, NULL, env, node, assignment_expression(env));
			node->length = ++length;
		} else {
			return node;
		}
	}
}


Node *primary_expression(Env *env) {
	if (consume('(')) {
		Node *node = additive_expression(env); // TODO: NEED TO CHANGE
		err_consume(')', "there isn't right-parenthesis at primary_expression");
		return node;
	}

	char *t_name;
			
	switch (((Token *)vec_get(tokens, pos))->ty){
	case TK_NUM:
		return new_node_num(((Token *)vec_get(tokens, pos++))->val, env);
		break;
	case TK_IDENT:
		t_name = ((Token *)vec_get(tokens, pos++))->input;
		/*if (consume('(')) {
			Vector *args = new_vector();
			while (1) {
				if (consume(')')) {
					break;
				}
				vec_push(args, assign(env));
				if (!consume(',')) {
					err_consume(')', "no right-parenthesis at func_call");
					break;
				}
			}
			return new_node_func(t_name, args, env);
		} else if (consume('[')) {
			// a[3]; -> *(a + 3);			
			Node *rhs = add(env);
			Node *lhs = new_node_ident(t_name, env);
			Node *plus = new_node('+', new_type(TY_PTR), env, lhs, rhs);
			Node *node = new_node(ND_DEREF, new_type(lhs->value_ty->ptrof->ty), env, plus, NULL);
			err_consume(']', "no right-bracket at array_suffix");
			return node;
		} else {*/
		Node *node = new_node_ident(t_name, env);
		return node;
		//}
		break;
	}
	
	return NULL;
}
