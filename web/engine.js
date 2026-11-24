/**
 * CineBrew Web Engine
 * Full-fidelity JavaScript port of CineBrew Compiler & Stack-Based VM
 * Matches C++ implementation in src/compiler and src/vm
 */

// ============================================================================
// 1. TOKEN TYPES & LEXER
// ============================================================================

const TokenType = {
    // Keywords (Cinema + Coffee Theme)
    TAKE: 'TAKE',
    POUR: 'POUR',
    SCENE: 'SCENE',
    SHOT: 'SHOT',
    IF: 'IF',
    ELSE: 'ELSE',
    LOOP: 'LOOP',
    BREAK: 'BREAK',
    CONTINUE: 'CONTINUE',

    // Literals
    IDENTIFIER: 'IDENTIFIER',
    NUMBER: 'NUMBER',
    STRING: 'STRING',
    TRUE_KW: 'TRUE_KW',
    FALSE_KW: 'FALSE_KW',

    // Operators
    PLUS: 'PLUS',
    MINUS: 'MINUS',
    STAR: 'STAR',
    SLASH: 'SLASH',
    EQUAL: 'EQUAL',
    EQUAL_EQUAL: 'EQUAL_EQUAL',
    BANG_EQUAL: 'BANG_EQUAL',
    GREATER: 'GREATER',
    GREATER_EQUAL: 'GREATER_EQUAL',
    LESS: 'LESS',
    LESS_EQUAL: 'LESS_EQUAL',

    // Punctuation
    SEMICOLON: 'SEMICOLON',
    LPAREN: 'LPAREN',
    RPAREN: 'RPAREN',
    LBRACE: 'LBRACE',
    RBRACE: 'RBRACE',
    COMMA: 'COMMA',

    // Special
    END_OF_FILE: 'END_OF_FILE',
    ERROR: 'ERROR'
};

const KEYWORDS = {
    'TAKE': TokenType.TAKE,
    'POUR': TokenType.POUR,
    'SCENE': TokenType.SCENE,
    'SHOT': TokenType.SHOT,
    'IF': TokenType.IF,
    'ELSE': TokenType.ELSE,
    'LOOP': TokenType.LOOP,
    'BREAK': TokenType.BREAK,
    'CONTINUE': TokenType.CONTINUE,
    'true': TokenType.TRUE_KW,
    'false': TokenType.FALSE_KW
};

class Lexer {
    constructor(source) {
        this.source = source;
        this.tokens = [];
        this.start = 0;
        this.current = 0;
        this.line = 1;
        this.hadError = false;
        this.errorMessage = '';
    }

    tokenize() {
        while (!this.isAtEnd()) {
            this.start = this.current;
            this.scanToken();
        }
        this.tokens.push({ type: TokenType.END_OF_FILE, lexeme: '', line: this.line, literal: null });
        return this.tokens;
    }

    isAtEnd() {
        return this.current >= this.source.length;
    }

    advance() {
        return this.source[this.current++];
    }

    peek() {
        if (this.isAtEnd()) return '\0';
        return this.source[this.current];
    }

    peekNext() {
        if (this.current + 1 >= this.source.length) return '\0';
        return this.source[this.current + 1];
    }

    match(expected) {
        if (this.isAtEnd() || this.source[this.current] !== expected) return false;
        this.current++;
        return true;
    }

    addToken(type, literal = null) {
        const text = this.source.substring(this.start, this.current);
        this.tokens.push({ type, lexeme: text, line: this.line, literal });
    }

    scanToken() {
        const c = this.advance();
        switch (c) {
            case '(': this.addToken(TokenType.LPAREN); break;
            case ')': this.addToken(TokenType.RPAREN); break;
            case '{': this.addToken(TokenType.LBRACE); break;
            case '}': this.addToken(TokenType.RBRACE); break;
            case ';': this.addToken(TokenType.SEMICOLON); break;
            case ',': this.addToken(TokenType.COMMA); break;
            case '+': this.addToken(TokenType.PLUS); break;
            case '-': this.addToken(TokenType.MINUS); break;
            case '*': this.addToken(TokenType.STAR); break;
            case '/': this.addToken(TokenType.SLASH); break;
            case '=':
                this.addToken(this.match('=') ? TokenType.EQUAL_EQUAL : TokenType.EQUAL);
                break;
            case '!':
                if (this.match('=')) {
                    this.addToken(TokenType.BANG_EQUAL);
                } else {
                    this.error(`Unexpected character '!'`);
                }
                break;
            case '<':
                this.addToken(this.match('=') ? TokenType.LESS_EQUAL : TokenType.LESS);
                break;
            case '>':
                this.addToken(this.match('=') ? TokenType.GREATER_EQUAL : TokenType.GREATER);
                break;
            case '#':
                // Single-line comment
                while (this.peek() !== '\n' && !this.isAtEnd()) this.advance();
                break;
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                this.line++;
                break;
            case '"':
                this.stringLiteral();
                break;
            default:
                if (this.isDigit(c)) {
                    this.numberLiteral();
                } else if (this.isAlpha(c)) {
                    this.identifier();
                } else {
                    this.error(`Unexpected character '${c}'`);
                }
                break;
        }
    }

    stringLiteral() {
        while (this.peek() !== '"' && !this.isAtEnd()) {
            if (this.peek() === '\n') this.line++;
            this.advance();
        }
        if (this.isAtEnd()) {
            this.error('Unterminated string.');
            return;
        }
        this.advance(); // closing quote
        const val = this.source.substring(this.start + 1, this.current - 1);
        this.addToken(TokenType.STRING, val);
    }

    numberLiteral() {
        while (this.isDigit(this.peek())) this.advance();
        const val = this.source.substring(this.start, this.current);
        this.addToken(TokenType.NUMBER, parseInt(val, 10));
    }

    identifier() {
        while (this.isAlphaNumeric(this.peek())) this.advance();
        const text = this.source.substring(this.start, this.current);
        const type = KEYWORDS[text] || TokenType.IDENTIFIER;
        this.addToken(type, text);
    }

    isDigit(c) {
        return c >= '0' && c <= '9';
    }

    isAlpha(c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c === '_';
    }

    isAlphaNumeric(c) {
        return this.isAlpha(c) || this.isDigit(c);
    }

    error(message) {
        if (this.hadError) return;
        this.hadError = true;
        this.errorMessage = `Line ${this.line}: ${message}`;
    }
}

// ============================================================================
// 2. PARSER & AST
// ============================================================================

class Parser {
    constructor(tokens) {
        this.tokens = tokens;
        this.current = 0;
        this.hadError = false;
        this.errorMessage = '';
    }

    parse() {
        const statements = [];
        while (!this.isAtEnd()) {
            const stmt = this.declaration();
            if (stmt) statements.push(stmt);
            if (this.hadError) break;
        }
        return { type: 'Program', statements };
    }

    isAtEnd() {
        return this.peek().type === TokenType.END_OF_FILE;
    }

    peek() {
        return this.tokens[this.current];
    }

    previous() {
        return this.tokens[this.current - 1];
    }

    advance() {
        if (!this.isAtEnd()) this.current++;
        return this.previous();
    }

    check(type) {
        if (this.isAtEnd()) return false;
        return this.peek().type === type;
    }

    match(...types) {
        for (const t of types) {
            if (this.check(t)) {
                this.advance();
                return true;
            }
        }
        return false;
    }

    consume(type, message) {
        if (this.check(type)) return this.advance();
        this.error(this.peek(), message);
        return this.peek();
    }

    error(token, message) {
        if (this.hadError) return;
        this.hadError = true;
        this.errorMessage = `Line ${token.line}: ${message}`;
    }

    declaration() {
        if (this.match(TokenType.SCENE)) return this.functionStmt();
        if (this.match(TokenType.TAKE)) return this.declarationStmt();
        return this.statement();
    }

    functionStmt() {
        const keyword = this.previous();
        const name = this.consume(TokenType.IDENTIFIER, "Expected function name");
        this.consume(TokenType.LPAREN, "Expected '(' after function name");
        const parameters = [];
        if (!this.check(TokenType.RPAREN)) {
            do {
                parameters.push(this.consume(TokenType.IDENTIFIER, "Expected parameter name"));
            } while (this.match(TokenType.COMMA));
        }
        this.consume(TokenType.RPAREN, "Expected ')' after parameters");
        this.consume(TokenType.LBRACE, "Expected '{' before function body");
        const body = this.block();
        return { type: 'FunctionStmt', name: name.lexeme, parameters: parameters.map(p => p.lexeme), body };
    }

    declarationStmt() {
        const name = this.consume(TokenType.IDENTIFIER, "Expected variable name");
        this.consume(TokenType.EQUAL, "Expected '=' after variable name");
        const initializer = this.expression();
        this.consume(TokenType.SEMICOLON, "Expected ';' after variable declaration");
        return { type: 'DeclarationStmt', name: name.lexeme, initializer };
    }

    statement() {
        if (this.match(TokenType.POUR)) return this.printStmt();
        if (this.match(TokenType.IF)) return this.ifStmt();
        if (this.match(TokenType.LOOP)) return this.loopStmt();
        if (this.match(TokenType.BREAK)) return this.breakStmt();
        if (this.match(TokenType.CONTINUE)) return this.continueStmt();
        if (this.match(TokenType.SHOT)) return this.returnStmt();
        if (this.match(TokenType.LBRACE)) return this.block();
        return this.assignmentStmt();
    }

    block() {
        const statements = [];
        while (!this.check(TokenType.RBRACE) && !this.isAtEnd()) {
            statements.push(this.declaration());
        }
        this.consume(TokenType.RBRACE, "Expected '}' after block");
        return { type: 'BlockStmt', statements };
    }

    printStmt() {
        const expr = this.expression();
        this.consume(TokenType.SEMICOLON, "Expected ';' after POUR statement");
        return { type: 'PrintStmt', expression: expr };
    }

    ifStmt() {
        const condition = this.expression();
        this.consume(TokenType.LBRACE, "Expected '{' after IF condition");
        const thenBranch = this.block();
        let elseBranch = null;
        if (this.match(TokenType.ELSE)) {
            this.consume(TokenType.LBRACE, "Expected '{' after ELSE");
            elseBranch = this.block();
        }
        return { type: 'IfStmt', condition, thenBranch, elseBranch };
    }

    loopStmt() {
        const condition = this.expression();
        this.consume(TokenType.LBRACE, "Expected '{' after LOOP condition");
        const body = this.block();
        return { type: 'LoopStmt', condition, body };
    }

    breakStmt() {
        this.consume(TokenType.SEMICOLON, "Expected ';' after BREAK");
        return { type: 'BreakStmt' };
    }

    continueStmt() {
        this.consume(TokenType.SEMICOLON, "Expected ';' after CONTINUE");
        return { type: 'ContinueStmt' };
    }

    returnStmt() {
        let value = null;
        if (!this.check(TokenType.SEMICOLON)) {
            value = this.expression();
        }
        this.consume(TokenType.SEMICOLON, "Expected ';' after SHOT statement");
        return { type: 'ReturnStmt', value };
    }

    assignmentStmt() {
        const next = this.tokens[this.current + 1];
        if (this.check(TokenType.IDENTIFIER) && next && next.type === TokenType.EQUAL) {
            const name = this.advance();
            this.advance(); // consume '='
            const value = this.expression();
            this.consume(TokenType.SEMICOLON, "Expected ';' after assignment");
            return { type: 'AssignmentStmt', name: name.lexeme, value };
        }

        const expr = this.expression();
        this.consume(TokenType.SEMICOLON, "Expected ';' after expression");
        return { type: 'ExpressionStmt', expression: expr };
    }

    expression() {
        return this.comparison();
    }

    comparison() {
        let expr = this.addition();
        while (this.match(TokenType.GREATER, TokenType.GREATER_EQUAL, TokenType.LESS, TokenType.LESS_EQUAL, TokenType.EQUAL_EQUAL, TokenType.BANG_EQUAL)) {
            const op = this.previous().type;
            const right = this.addition();
            expr = { type: 'BinaryExpr', op, left: expr, right };
        }
        return expr;
    }

    addition() {
        let expr = this.multiplication();
        while (this.match(TokenType.PLUS, TokenType.MINUS)) {
            const op = this.previous().type;
            const right = this.multiplication();
            expr = { type: 'BinaryExpr', op, left: expr, right };
        }
        return expr;
    }

    multiplication() {
        let expr = this.unary();
        while (this.match(TokenType.STAR, TokenType.SLASH)) {
            const op = this.previous().type;
            const right = this.unary();
            expr = { type: 'BinaryExpr', op, left: expr, right };
        }
        return expr;
    }

    unary() {
        if (this.match(TokenType.MINUS)) {
            const right = this.unary();
            return { type: 'UnaryExpr', op: 'MINUS', right };
        }
        return this.primary();
    }

    primary() {
        if (this.match(TokenType.NUMBER)) {
            return { type: 'LiteralExpr', value: this.previous().literal };
        }
        if (this.match(TokenType.TRUE_KW)) {
            return { type: 'LiteralExpr', value: 1 };
        }
        if (this.match(TokenType.FALSE_KW)) {
            return { type: 'LiteralExpr', value: 0 };
        }
        if (this.match(TokenType.STRING)) {
            return { type: 'StringLiteralExpr', value: this.previous().literal };
        }
        if (this.match(TokenType.IDENTIFIER)) {
            const name = this.previous();
            if (this.match(TokenType.LPAREN)) {
                return this.finishCall(name);
            }
            return { type: 'VariableExpr', name: name.lexeme };
        }
        if (this.match(TokenType.LPAREN)) {
            const expr = this.expression();
            this.consume(TokenType.RPAREN, "Expected ')' after expression");
            return expr;
        }

        this.error(this.peek(), `Expected expression, got '${this.peek().lexeme}'`);
        return { type: 'LiteralExpr', value: 0 };
    }

    finishCall(callee) {
        const args = [];
        if (!this.check(TokenType.RPAREN)) {
            do {
                args.push(this.expression());
            } while (this.match(TokenType.COMMA));
        }
        this.consume(TokenType.RPAREN, "Expected ')' after arguments");
        return { type: 'CallExpr', callee: callee.lexeme, arguments: args };
    }
}

// ============================================================================
// 3. CODE GENERATOR
// ============================================================================

class CodeGenerator {
    constructor() {
        this.bytecode = [];
        this.labelCount = 0;
        this.loopBreakLabels = [];
        this.loopContinueLabels = [];
        this.hadError = false;
        this.errorMessage = '';
    }

    newLabel(prefix = 'L') {
        return `${prefix}_${this.labelCount++}`;
    }

    emit(instr) {
        this.bytecode.push(instr);
    }

    generate(program) {
        this.bytecode = [];
        this.labelCount = 0;
        this.loopBreakLabels = [];
        this.loopContinueLabels = [];

        const startLabel = this.newLabel('main_entry');
        this.emit(`JMP ${startLabel}`);

        const functions = [];
        const mainStmts = [];

        for (const stmt of program.statements) {
            if (stmt.type === 'FunctionStmt') {
                functions.push(stmt);
            } else {
                mainStmts.push(stmt);
            }
        }

        for (const func of functions) {
            this.visitFunction(func);
        }

        this.emit(`${startLabel}:`);
        for (const stmt of mainStmts) {
            this.visitStmt(stmt);
        }

        this.emit('HALT:');
        return this.bytecode;
    }

    visitFunction(func) {
        this.emit(`${func.name}:`);
        for (let i = 0; i < func.parameters.length; i++) {
            this.emit(`LOADARG ${i}`);
            this.emit(`STORE ${func.parameters[i]}`);
        }
        this.visitBlock(func.body);
        this.emit('PUSH 0');
        this.emit('RET');
    }

    visitBlock(block) {
        for (const stmt of block.statements) {
            this.visitStmt(stmt);
        }
    }

    visitStmt(stmt) {
        if (!stmt) return;
        switch (stmt.type) {
            case 'DeclarationStmt':
                this.visitExpr(stmt.initializer);
                this.emit(`STORE ${stmt.name}`);
                break;
            case 'AssignmentStmt':
                this.visitExpr(stmt.value);
                this.emit(`STORE ${stmt.name}`);
                break;
            case 'PrintStmt':
                if (stmt.expression.type === 'StringLiteralExpr') {
                    this.emit(`PUSH "${stmt.expression.value}"`);
                    this.emit('PRINT');
                } else {
                    this.visitExpr(stmt.expression);
                    this.emit('PRINT');
                }
                break;
            case 'IfStmt': {
                const elseLabel = this.newLabel('else');
                const endLabel = this.newLabel('endif');
                this.visitExpr(stmt.condition);
                this.emit(`JZ ${stmt.elseBranch ? elseLabel : endLabel}`);
                this.visitBlock(stmt.thenBranch);
                if (stmt.elseBranch) {
                    this.emit(`JMP ${endLabel}`);
                    this.emit(`${elseLabel}:`);
                    this.visitBlock(stmt.elseBranch);
                }
                this.emit(`${endLabel}:`);
                break;
            }
            case 'LoopStmt': {
                const startLabel = this.newLabel('loop_start');
                const endLabel = this.newLabel('loop_end');
                this.loopBreakLabels.push(endLabel);
                this.loopContinueLabels.push(startLabel);

                this.emit(`${startLabel}:`);
                this.visitExpr(stmt.condition);
                this.emit(`JZ ${endLabel}`);
                this.visitBlock(stmt.body);
                this.emit(`JMP ${startLabel}`);
                this.emit(`${endLabel}:`);

                this.loopBreakLabels.pop();
                this.loopContinueLabels.pop();
                break;
            }
            case 'BreakStmt':
                if (this.loopBreakLabels.length > 0) {
                    this.emit(`JMP ${this.loopBreakLabels[this.loopBreakLabels.length - 1]}`);
                }
                break;
            case 'ContinueStmt':
                if (this.loopContinueLabels.length > 0) {
                    this.emit(`JMP ${this.loopContinueLabels[this.loopContinueLabels.length - 1]}`);
                }
                break;
            case 'ReturnStmt':
                if (stmt.value) {
                    this.visitExpr(stmt.value);
                } else {
                    this.emit('PUSH 0');
                }
                this.emit('RET');
                break;
            case 'ExpressionStmt':
                this.visitExpr(stmt.expression);
                // CineBrew VM doesn't have explicit POP, top item remains on stack or is absorbed
                break;
        }
    }

    visitExpr(expr) {
        if (!expr) return;
        switch (expr.type) {
            case 'LiteralExpr':
                this.emit(`PUSH ${expr.value}`);
                break;
            case 'StringLiteralExpr':
                this.emit(`PUSH "${expr.value}"`);
                break;
            case 'VariableExpr':
                this.emit(`LOAD ${expr.name}`);
                break;
            case 'UnaryExpr':
                if (expr.op === 'MINUS') {
                    this.emit('PUSH 0');
                    this.visitExpr(expr.right);
                    this.emit('SUB');
                }
                break;
            case 'BinaryExpr':
                this.visitExpr(expr.left);
                this.visitExpr(expr.right);
                switch (expr.op) {
                    case TokenType.PLUS: this.emit('ADD'); break;
                    case TokenType.MINUS: this.emit('SUB'); break;
                    case TokenType.STAR: this.emit('MUL'); break;
                    case TokenType.SLASH: this.emit('DIV'); break;
                    case TokenType.EQUAL_EQUAL: this.emit('EQ'); break;
                    case TokenType.BANG_EQUAL: this.emit('NE'); break;
                    case TokenType.GREATER: this.emit('GT'); break;
                    case TokenType.LESS: this.emit('LT'); break;
                    case TokenType.GREATER_EQUAL:
                        // a >= b <=> (a > b) || (a == b) or !(a < b)
                        this.emit('LT');
                        this.emit('PUSH 0');
                        this.emit('EQ');
                        break;
                    case TokenType.LESS_EQUAL:
                        // a <= b <=> !(a > b)
                        this.emit('GT');
                        this.emit('PUSH 0');
                        this.emit('EQ');
                        break;
                }
                break;
            case 'CallExpr':
                for (const arg of expr.arguments) {
                    this.visitExpr(arg);
                }
                this.emit(`CALL ${expr.callee} ${expr.arguments.length}`);
                break;
        }
    }
}

// ============================================================================
// 4. CINEBREW VIRTUAL MACHINE
// ============================================================================

class VM {
    constructor(canvas = null, onPrint = null) {
        this.canvas = canvas;
        this.ctx = canvas ? canvas.getContext('2d') : null;
        this.onPrint = onPrint || ((msg) => console.log(msg));

        this.stack = [];
        this.vars = {};
        this.pc = 0;
        this.labels = {};
        this.callstack = [];
        this.keys = {};
        this.currentColor = '#ffffff';

        // Set up keyboard listeners if canvas present
        if (typeof window !== 'undefined') {
            window.addEventListener('keydown', (e) => {
                const code = this.mapDOMKey(e);
                if (code) {
                    this.keys[code] = true;
                    // Prevent page scroll on arrows/space if focused
                    if ([80, 82, 79, 81, 44].includes(code)) e.preventDefault();
                }
            });
            window.addEventListener('keyup', (e) => {
                const code = this.mapDOMKey(e);
                if (code) this.keys[code] = false;
            });
        }
    }

    mapDOMKey(e) {
        switch (e.key) {
            case 'ArrowLeft': return 80;
            case 'ArrowRight': return 79;
            case 'ArrowUp': return 82;
            case 'ArrowDown': return 81;
            case ' ': return 44;
            case 'Enter': return 40;
            case 'Escape': return 41;
            case 'w': case 'W': return 26;
            case 'a': case 'A': return 4;
            case 's': case 'S': return 22;
            case 'd': case 'D': return 7;
            case 'r': case 'R': return 21;
            default:
                if (e.key >= '0' && e.key <= '9') return 30 + (parseInt(e.key) || 9);
                return 0;
        }
    }

    reset() {
        this.stack = [];
        this.vars = {};
        this.pc = 0;
        this.labels = {};
        this.callstack = [];
    }

    preprocess(program) {
        this.labels = {};
        for (let i = 0; i < program.length; i++) {
            const line = program[i].trim();
            if (line.endsWith(':')) {
                const labelName = line.slice(0, -1);
                this.labels[labelName] = i;
            }
        }
    }

    push(val) {
        this.stack.push(val);
    }

    pop() {
        if (this.stack.length === 0) return 0;
        return this.stack.pop();
    }

    executeInstruction(instruction, program) {
        const trimmed = instruction.trim();
        if (!trimmed || trimmed.endsWith(':')) {
            this.pc++;
            return;
        }

        // Parse instruction
        const parts = [];
        let cur = '';
        let inQuote = false;
        for (let i = 0; i < trimmed.length; i++) {
            const c = trimmed[i];
            if (c === '"') {
                inQuote = !inQuote;
                cur += c;
            } else if (/\s/.test(c) && !inQuote) {
                if (cur) { parts.push(cur); cur = ''; }
            } else {
                cur += c;
            }
        }
        if (cur) parts.push(cur);

        const opcode = parts[0];

        switch (opcode) {
            case 'PUSH': {
                const raw = parts[1];
                if (raw && raw.startsWith('"') && raw.endsWith('"')) {
                    // String literal
                    this.push(raw.slice(1, -1));
                } else {
                    this.push(parseInt(raw, 10) || 0);
                }
                this.pc++;
                break;
            }
            case 'ADD': {
                const b = this.pop();
                const a = this.pop();
                this.push(a + b);
                this.pc++;
                break;
            }
            case 'SUB': {
                const b = this.pop();
                const a = this.pop();
                this.push(a - b);
                this.pc++;
                break;
            }
            case 'MUL': {
                const b = this.pop();
                const a = this.pop();
                this.push(a * b);
                this.pc++;
                break;
            }
            case 'DIV': {
                const b = this.pop();
                const a = this.pop();
                this.push(b === 0 ? 0 : Math.floor(a / b));
                this.pc++;
                break;
            }
            case 'STORE': {
                const varName = parts[1];
                this.vars[varName] = this.pop();
                this.pc++;
                break;
            }
            case 'LOAD': {
                const varName = parts[1];
                this.push(this.vars[varName] !== undefined ? this.vars[varName] : 0);
                this.pc++;
                break;
            }
            case 'EQ': {
                const b = this.pop();
                const a = this.pop();
                this.push(a === b ? 1 : 0);
                this.pc++;
                break;
            }
            case 'NE': {
                const b = this.pop();
                const a = this.pop();
                this.push(a !== b ? 1 : 0);
                this.pc++;
                break;
            }
            case 'GT': {
                const b = this.pop();
                const a = this.pop();
                this.push(a > b ? 1 : 0);
                this.pc++;
                break;
            }
            case 'LT': {
                const b = this.pop();
                const a = this.pop();
                this.push(a < b ? 1 : 0);
                this.pc++;
                break;
            }
            case 'JMP': {
                const target = parts[1];
                this.pc = this.labels[target] !== undefined ? this.labels[target] : this.pc + 1;
                break;
            }
            case 'JZ': {
                const target = parts[1];
                const val = this.pop();
                if (val === 0 && this.labels[target] !== undefined) {
                    this.pc = this.labels[target];
                } else {
                    this.pc++;
                }
                break;
            }
            case 'JNZ': {
                const target = parts[1];
                const val = this.pop();
                if (val !== 0 && this.labels[target] !== undefined) {
                    this.pc = this.labels[target];
                } else {
                    this.pc++;
                }
                break;
            }
            case 'PRINT': {
                const val = this.pop();
                this.onPrint(val);
                this.pc++;
                break;
            }
            case 'CALL': {
                const label = parts[1];
                const argc = parseInt(parts[2], 10) || 0;

                // Builtin functions check
                if (this.isBuiltin(label)) {
                    const args = [];
                    for (let i = 0; i < argc; i++) {
                        args.unshift(this.pop());
                    }
                    const res = this.callBuiltin(label, args);
                    this.push(res);
                    this.pc++;
                } else {
                    const argStartIndex = this.stack.length - argc;
                    this.callstack.push({
                        returnPc: this.pc + 1,
                        prevStackSize: argStartIndex,
                        argCount: argc
                    });
                    if (this.labels[label] !== undefined) {
                        this.pc = this.labels[label];
                    } else {
                        this.pc++;
                    }
                }
                break;
            }
            case 'LOADARG': {
                const argIdx = parseInt(parts[1], 10) || 0;
                if (this.callstack.length > 0) {
                    const frame = this.callstack[this.callstack.length - 1];
                    const pos = frame.prevStackSize + argIdx;
                    this.push(this.stack[pos] !== undefined ? this.stack[pos] : 0);
                } else {
                    this.push(0);
                }
                this.pc++;
                break;
            }
            case 'RET': {
                if (this.callstack.length === 0) {
                    this.pc = program.length;
                    return;
                }
                const retVal = this.stack.length > 0 ? this.pop() : 0;
                const frame = this.callstack.pop();
                this.stack.length = frame.prevStackSize;
                this.push(retVal);
                this.pc = frame.returnPc;
                break;
            }
            default:
                this.pc++;
                break;
        }
    }

    isBuiltin(name) {
        return [
            'print', 'input', 'random', 'time', 'abs', 'min', 'max',
            'keyPressed', 'getScreenWidth', 'getScreenHeight',
            'clearScreen', 'setColor', 'drawRectangle', 'drawCircle', 'drawLine'
        ].includes(name);
    }

    callBuiltin(name, args) {
        switch (name) {
            case 'print':
                this.onPrint(args[0] !== undefined ? args[0] : '');
                return 0;
            case 'abs':
                return Math.abs(args[0] || 0);
            case 'min':
                return Math.min(args[0] || 0, args[1] || 0);
            case 'max':
                return Math.max(args[0] || 0, args[1] || 0);
            case 'random':
                if (args.length === 1) {
                    const m = args[0] || 1;
                    return Math.floor(Math.random() * m);
                } else if (args.length >= 2) {
                    const min = args[0], max = args[1];
                    return min + Math.floor(Math.random() * (max - min));
                }
                return Math.floor(Math.random() * 100);
            case 'time':
                return Math.floor(Date.now() / 1000);
            case 'keyPressed': {
                const key = args[0];
                return this.keys[key] ? 1 : 0;
            }
            case 'getScreenWidth':
                return this.canvas ? this.canvas.width : 800;
            case 'getScreenHeight':
                return this.canvas ? this.canvas.height : 600;
            case 'clearScreen':
                if (this.ctx) {
                    this.ctx.fillStyle = '#0f172a';
                    this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
                }
                return 0;
            case 'setColor':
                if (this.ctx && args.length >= 3) {
                    this.currentColor = `rgb(${args[0]}, ${args[1]}, ${args[2]})`;
                    this.ctx.fillStyle = this.currentColor;
                    this.ctx.strokeStyle = this.currentColor;
                }
                return 0;
            case 'drawRectangle':
                if (this.ctx && args.length >= 4) {
                    this.ctx.fillStyle = this.currentColor;
                    this.ctx.fillRect(args[0], args[1], args[2], args[3]);
                }
                return 0;
            case 'drawCircle':
                if (this.ctx && args.length >= 3) {
                    this.ctx.fillStyle = this.currentColor;
                    this.ctx.beginPath();
                    this.ctx.arc(args[0], args[1], args[2], 0, Math.PI * 2);
                    this.ctx.fill();
                }
                return 0;
            case 'drawLine':
                if (this.ctx && args.length >= 4) {
                    this.ctx.strokeStyle = this.currentColor;
                    this.ctx.beginPath();
                    this.ctx.moveTo(args[0], args[1]);
                    this.ctx.lineTo(args[2], args[3]);
                    this.ctx.stroke();
                }
                return 0;
            default:
                return 0;
        }
    }

    callFunction(functionName, program) {
        if (this.labels[functionName] === undefined) return;
        const savedPC = this.pc;
        const savedStackSize = this.stack.length;

        // Issue call
        const argStartIndex = this.stack.length;
        this.callstack.push({
            returnPc: program.length,
            prevStackSize: argStartIndex,
            argCount: 0
        });
        this.pc = this.labels[functionName];

        let steps = 0;
        const maxSteps = 20000;
        while (this.pc < program.length && steps < maxSteps) {
            if (this.callstack.length === 0) break;
            const instr = program[this.pc];
            this.executeInstruction(instr, program);
            steps++;
        }

        this.pc = savedPC;
        this.stack.length = savedStackSize;
    }

    run(program, maxInstructions = 50000) {
        this.preprocess(program);
        this.pc = 0;
        let count = 0;
        while (this.pc < program.length && count < maxInstructions) {
            const instr = program[this.pc];
            if (instr === 'HALT:') break;
            this.executeInstruction(instr, program);
            count++;
        }
    }
}

// Export for Node and Browser environments
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { Lexer, Parser, CodeGenerator, VM, TokenType };
}

